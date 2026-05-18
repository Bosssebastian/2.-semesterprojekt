#!/usr/bin/env python3
import argparse
import csv
from collections import deque
from pathlib import Path
import queue
import sys
import threading
import time

try:
    import curses
except ImportError:
    curses = None

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("pyserial is required. Install it with: py -m pip install pyserial", file=sys.stderr)
    raise SystemExit(1)


BAUDRATE = 115200
TIMEOUT_SECONDS = 1.0
DEFAULT_STALL_LOG = "stall_values.csv"
DEFAULT_STALL_MODE = "auto"
DEFAULT_MOVE_PAUSE_SECONDS = 0.05
CLOSE_DONE = "EVENT MOVE_DONE CLOSE"
OPEN_MOVE_DONE = "EVENT MOVE_DONE OPEN"
OPEN_RESET_DONE = "EVENT MOVE_DONE OPEN_RESET"
OPEN_RESET_FORWARD_DONE = "EVENT MOVE_DONE OPEN_RESET_FORWARD"

COMMANDS = {
    "PING",
    "OPEN",
    "CLOSE",
    "STOP",
    "STATUS",
    "STATISTICS",
    "RESET",
    "CURRENT_EVENTS_ON",
    "CURRENT_EVENTS_OFF",
    "STALL_VALUES_ON",
    "STALL_VALUES_OFF",
}
SETUP_COMMANDS = ("CURRENT_EVENTS_OFF", "STALL_VALUES_ON")
RESTORE_COMMANDS = ("STALL_VALUES_OFF", "CURRENT_EVENTS_ON")


class StallMoveLogger:
    def __init__(self, log_path, active_mode):
        self.log_path = Path(log_path) if log_path else None
        self.active_mode = active_mode.lower()
        self.active_command = None
        self.move_mode = None
        self.values = []
        self.lock = threading.Lock()

        if self.log_path is not None:
            self.log_path.parent.mkdir(parents=True, exist_ok=True)
            with self.log_path.open("w", newline="", encoding="utf-8") as log_file:
                csv.writer(log_file).writerow(["event", "move", "result", "stall_mode", "values"])

    def start_command(self, command):
        with self.lock:
            self.active_command = command
            self.move_mode = None
            self.values = []

    def add_serial_line(self, line):
        parts = line.split()
        if len(parts) < 4 or parts[0] != "EVENT" or parts[1] != "STALL_VALUE":
            return
        mode = parts[2].lower()
        if self.active_mode != "auto" and mode != self.active_mode:
            return
        with self.lock:
            if self.active_mode == "auto":
                if self.move_mode is None:
                    self.move_mode = mode
                elif mode != self.move_mode:
                    return
            self.values.append(parts[3])

    def finish_event(self, line):
        parts = line.split()
        if len(parts) < 4 or parts[0] != "EVENT":
            return

        event = parts[1]
        move = parts[2] if len(parts) >= 3 else ""
        result = parts[3] if len(parts) >= 4 else ""

        with self.lock:
            active_command = self.active_command
            move_mode = self.move_mode
            values = self.values
            self.move_mode = None
            self.values = []

            if event == "OPEN_SEQUENCE_DONE" or move != "OPEN" or result in {"STOPPED", "MOVE_ERROR"}:
                self.active_command = None
            elif active_command is None:
                self.active_command = move

        if self.log_path is not None:
            with self.log_path.open("a", newline="", encoding="utf-8") as log_file:
                csv.writer(log_file).writerow([event, move, result, move_mode or "", " ".join(values)])

    def clear(self):
        with self.lock:
            self.active_command = None
            self.move_mode = None
            self.values = []
        if self.log_path is not None:
            with self.log_path.open("w", newline="", encoding="utf-8") as log_file:
                csv.writer(log_file).writerow(["event", "move", "result", "stall_mode", "values"])


class GripperTui:
    def __init__(self, ser, stall_logger, move_pause, show_stall_values):
        self.ser = ser
        self.stall_logger = stall_logger
        self.move_pause = move_pause
        self.show_stall_values = show_stall_values
        self.logs = deque(maxlen=2000)
        self.logs_lock = threading.Lock()
        self.event_condition = threading.Condition()
        self.event_counter = 0
        self.events = deque(maxlen=500)
        self.incoming = queue.Queue()
        self.stop_event = threading.Event()
        self.auto_cancel = threading.Event()
        self.auto_thread = None
        self.auto_good_runs = 0
        self.auto_failed_runs = 0
        self.auto_attempted_runs = 0
        self.reader_thread = threading.Thread(target=self.read_serial_loop, daemon=True)

    def add_log(self, line):
        with self.logs_lock:
            self.logs.append(line)

    def read_serial_loop(self):
        while not self.stop_event.is_set():
            try:
                raw = self.ser.readline()
            except (OSError, serial.SerialException):
                self.stop_event.set()
                return

            if not raw:
                continue

            line = raw.decode("utf-8", errors="replace").strip("\r\n")
            if not line or line.startswith("DEBUG "):
                continue
            self.incoming.put(line)

    def drain_incoming(self):
        while True:
            try:
                line = self.incoming.get_nowait()
            except queue.Empty:
                return

            self.stall_logger.add_serial_line(line)
            if self.show_stall_values or not line.startswith("EVENT STALL_VALUE "):
                self.add_log(line)
            with self.event_condition:
                self.event_counter += 1
                self.events.append((self.event_counter, line))
                self.event_condition.notify_all()
            if self.is_move_finished(line):
                self.stall_logger.finish_event(line)

    def send_command(self, command, log=True):
        command = command.strip().upper()
        if not command:
            return

        if command.startswith("CMD "):
            command = command[4:].strip().upper()

        package = f"CMD {command}"
        if command in {"OPEN", "CLOSE"}:
            self.stall_logger.start_command(command)

        try:
            self.ser.write(f"{package}\n".encode("utf-8"))
            self.ser.flush()
        except (OSError, serial.SerialException):
            self.stop_event.set()
            return

        if log:
            self.add_log(package)

    def handle_input(self, text):
        command = text.strip()
        if not command:
            return

        upper = command.upper()
        if upper in {"QUIT", "EXIT", "Q"}:
            self.stop_event.set()
            return
        if upper == "CLEAR":
            with self.logs_lock:
                self.logs.clear()
            self.stall_logger.clear()
            return
        if upper == "AUTO":
            self.start_auto()
            return
        if upper == "STOP" or upper == "CMD STOP":
            self.auto_cancel.set()

        if upper.startswith("CMD "):
            self.send_command(upper[4:])
            return
        if upper in COMMANDS:
            self.send_command(upper)
            return

    def start_auto(self):
        if self.auto_thread is not None and self.auto_thread.is_alive():
            return

        self.auto_cancel.clear()
        self.auto_thread = threading.Thread(target=self.run_auto_loop, daemon=True)
        self.auto_thread.start()

    def run_auto_loop(self):
        while not self.stop_event.is_set() and not self.auto_cancel.is_set():
            self.auto_attempted_runs += 1

            self.send_command("CLOSE")
            close_result = self.wait_for_expected_move(CLOSE_DONE, "STALL")
            if close_result is None:
                break

            if self.wait_pause():
                break

            self.send_command("OPEN")
            open_result = self.wait_for_open_moves()
            if open_result is None:
                break
            if close_result and open_result:
                self.auto_good_runs += 1
            else:
                self.auto_failed_runs += 1

            if self.wait_pause():
                break

        self.auto_cancel.clear()

    def wait_pause(self):
        deadline = time.monotonic() + self.move_pause
        while time.monotonic() < deadline:
            if self.stop_event.is_set() or self.auto_cancel.is_set():
                return True
            time.sleep(0.01)
        return False

    def wait_for_event(self, prefix):
        with self.event_condition:
            last_seen = self.event_counter

        while not self.stop_event.is_set() and not self.auto_cancel.is_set():
            with self.event_condition:
                self.event_condition.wait(timeout=0.1)
                for counter, line in self.events:
                    if counter <= last_seen:
                        continue
                    if line.startswith("EVENT ERROR "):
                        return False
                    if line.startswith(prefix):
                        return True
                last_seen = self.event_counter
        return False

    def wait_for_expected_move(self, prefix, expected_result):
        with self.event_condition:
            last_seen = self.event_counter

        while not self.stop_event.is_set() and not self.auto_cancel.is_set():
            with self.event_condition:
                self.event_condition.wait(timeout=0.1)
                for counter, line in self.events:
                    if counter <= last_seen:
                        continue
                    if line.startswith("EVENT ERROR "):
                        return None
                    if line.startswith(prefix):
                        return self.event_result(line) == expected_result
                last_seen = self.event_counter
        return None

    def wait_for_open_moves(self):
        expected = [
            (OPEN_MOVE_DONE, "STEPS_FINISHED"),
            (OPEN_RESET_DONE, "STALL"),
            (OPEN_RESET_FORWARD_DONE, "STEPS_FINISHED"),
        ]
        index = 0
        good = True

        with self.event_condition:
            last_seen = self.event_counter

        while not self.stop_event.is_set() and not self.auto_cancel.is_set():
            with self.event_condition:
                self.event_condition.wait(timeout=0.1)
                for counter, line in self.events:
                    if counter <= last_seen:
                        continue
                    if line.startswith("EVENT ERROR "):
                        return None
                    if not line.startswith("EVENT MOVE_DONE "):
                        continue

                    if index < len(expected):
                        expected_prefix, expected_result = expected[index]
                        if not line.startswith(expected_prefix) or self.event_result(line) != expected_result:
                            good = False
                        if line.startswith(expected_prefix):
                            index += 1
                    else:
                        good = False

                    if line.startswith(OPEN_RESET_FORWARD_DONE):
                        return good and index == len(expected)
                last_seen = self.event_counter
        return None

    @staticmethod
    def is_move_finished(line):
        return line.startswith("EVENT MOVE_DONE ") or line.startswith("EVENT ERROR ")

    @staticmethod
    def event_result(line):
        parts = line.split()
        return parts[-1] if parts else ""

    def run(self, stdscr):
        self.reader_thread.start()
        curses.curs_set(1)
        stdscr.nodelay(True)
        stdscr.keypad(True)

        input_text = ""
        while not self.stop_event.is_set():
            self.drain_incoming()
            self.draw(stdscr, input_text)

            try:
                key = stdscr.getch()
            except curses.error:
                key = -1

            if key == -1:
                time.sleep(0.03)
                continue
            if key in (10, 13):
                self.handle_input(input_text)
                input_text = ""
                continue
            if key in (curses.KEY_BACKSPACE, 127, 8):
                input_text = input_text[:-1]
                continue
            if key == curses.KEY_RESIZE:
                continue
            if 0 <= key <= 255 and chr(key).isprintable():
                input_text += chr(key)

        self.stop_event.set()

    def draw(self, stdscr, input_text):
        stdscr.erase()
        height, width = stdscr.getmaxyx()
        log_height = max(1, height - 3)

        with self.logs_lock:
            lines = list(self.logs)[-log_height:]

        for row, line in enumerate(lines):
            stdscr.addnstr(row, 0, line, width - 1)

        log_status = self.stall_logger.log_path if self.stall_logger.log_path is not None else "off"
        status = (
            "Type CMD OPEN, CMD CLOSE, CMD STOP, AUTO, CLEAR, QUIT"
            f" | AUTO good:{self.auto_good_runs} fail:{self.auto_failed_runs} attempted:{self.auto_attempted_runs}"
            f" | stall log:{log_status}"
        )
        stdscr.addnstr(height - 2, 0, "-" * max(0, width - 1), width - 1)
        stdscr.addnstr(height - 1, 0, f"{status} | > {input_text}", width - 1)
        stdscr.move(height - 1, min(width - 1, len(status) + 5 + len(input_text)))
        stdscr.refresh()


def open_serial(port, baudrate, timeout):
    ser = serial.Serial(
        port=port,
        baudrate=baudrate,
        timeout=0.1,
        write_timeout=timeout,
    )
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    return ser


def read_line(ser, timeout):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        raw = ser.readline()
        if not raw:
            continue
        line = raw.decode("utf-8", errors="replace").strip("\r\n")
        if line and not line.startswith("DEBUG "):
            return line
    return None


def probe_gripper(port, baudrate, timeout):
    ser = None
    try:
        ser = open_serial(port, baudrate, timeout)
        ser.write(b"CMD PING\n")
        ser.flush()
        return read_line(ser, timeout) == "OK PING GRIPPER"
    except (OSError, serial.SerialException):
        return False
    finally:
        if ser is not None:
            ser.close()


def auto_detect_port(baudrate, timeout):
    print("Scanning serial ports...")
    for port_info in list_ports.comports():
        print(f"Trying {port_info.device}...")
        if probe_gripper(port_info.device, baudrate, timeout):
            return port_info.device
    return None


def send_control_command(ser, command, timeout):
    ser.write(f"CMD {command}\n".encode("utf-8"))
    ser.flush()
    line = read_line(ser, timeout)
    return line is not None and line.startswith("OK ")


def apply_test_telemetry(ser, timeout):
    for command in SETUP_COMMANDS:
        send_control_command(ser, command, timeout)


def restore_default_telemetry(ser, timeout):
    for command in RESTORE_COMMANDS:
        try:
            send_control_command(ser, command, timeout)
        except (OSError, serial.SerialException):
            return


def main():
    parser = argparse.ArgumentParser(description="TUI USB serial test tool for the Pico gripper.")
    parser.add_argument("--port", help="COM port, for example COM5. If omitted, the script scans for the gripper.")
    parser.add_argument("--baud", type=int, default=BAUDRATE, help=f"Serial baud rate. Default: {BAUDRATE}")
    parser.add_argument("--timeout", type=float, default=TIMEOUT_SECONDS, help="Read timeout in seconds.")
    parser.add_argument(
        "--move-pause",
        type=float,
        default=DEFAULT_MOVE_PAUSE_SECONDS,
        help=f"Auto mode pause between individual moves, in seconds. Default: {DEFAULT_MOVE_PAUSE_SECONDS}",
    )
    parser.add_argument(
        "--stall-log",
        default=DEFAULT_STALL_LOG,
        help=f"CSV file for stall values. Each move is one row of values. Default: {DEFAULT_STALL_LOG}",
    )
    parser.add_argument(
        "--stall-mode",
        default=DEFAULT_STALL_MODE,
        help=f"Only log stall values with this mode. Default: {DEFAULT_STALL_MODE}",
    )
    parser.add_argument(
        "--no-stall-log",
        action="store_true",
        help="Do not write the per-move stall values CSV file.",
    )
    parser.add_argument(
        "--show-stall-values",
        action="store_true",
        help="Show EVENT STALL_VALUE messages in the TUI log. Hidden by default.",
    )
    args = parser.parse_args()

    if curses is None:
        print("curses is required for the TUI. On Windows, install it with: py -m pip install windows-curses", file=sys.stderr)
        return 1

    log_path = None if args.no_stall_log else args.stall_log
    stall_logger = StallMoveLogger(log_path, args.stall_mode)

    port = args.port or auto_detect_port(args.baud, args.timeout)
    if not port:
        print("Could not find gripper USB serial port.", file=sys.stderr)
        print("Try passing it manually, for example: py gripper_usb_test.py --port COM5", file=sys.stderr)
        return 1

    ser = None
    try:
        ser = open_serial(port, args.baud, args.timeout)
        apply_test_telemetry(ser, args.timeout)

        app = GripperTui(ser, stall_logger, args.move_pause, args.show_stall_values)
        curses.wrapper(app.run)
        return 0
    except (OSError, serial.SerialException) as exc:
        print(f"Serial error: {exc}", file=sys.stderr)
        return 1
    finally:
        if ser is not None:
            restore_default_telemetry(ser, args.timeout)
            ser.close()


if __name__ == "__main__":
    raise SystemExit(main())
