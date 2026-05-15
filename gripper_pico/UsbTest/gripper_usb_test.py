#!/usr/bin/env python3
import argparse
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("pyserial is required. Install it with: py -m pip install pyserial", file=sys.stderr)
    raise SystemExit(1)


BAUDRATE = 115200
TIMEOUT_SECONDS = 1.0

COMMANDS = {
    "ping": "PING",
    "open": "OPEN",
    "close": "CLOSE",
    "stop": "STOP",
    "status": "STATUS",
    "statistics": "STATISTICS",
}

SETUP_COMMANDS = ("CURRENT_EVENTS_OFF", "STALL_VALUES_ON")
RESTORE_COMMANDS = ("STALL_VALUES_OFF", "CURRENT_EVENTS_ON")


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
        if not line or line.startswith("DEBUG "):
            continue
        return line

    return None


def send_command(ser, command, verbose=True):
    package = f"CMD {command}\n"
    if verbose:
        print(f"> {package.rstrip()}")
    ser.write(package.encode("utf-8"))
    ser.flush()


def is_move_done_event(line):
    return line.startswith("EVENT MOVE_DONE ") or line.startswith("EVENT ERROR ")


def parse_stall_value(line):
    parts = line.split()
    if len(parts) < 4 or parts[0] != "EVENT" or parts[1] != "STALL_VALUE":
        return None
    return {
        "mode": parts[2],
        "value": parts[3],
        "line": line,
    }


def handle_background_line(line, stall_samples, print_non_data=True):
    if line.startswith("EVENT CURRENT "):
        return

    stall_value = parse_stall_value(line)
    if stall_value is not None:
        stall_samples.append(stall_value)
        return

    if print_non_data:
        print(f"< {line}")


def read_ack(ser, command, timeout, stall_samples=None, print_background=True, print_ack=True):
    while True:
        line = read_line(ser, timeout)
        if line is None:
            print(f"Timed out waiting for ACK from {command}", file=sys.stderr)
            return None

        if line.startswith("OK ") or line.startswith("ERROR "):
            if print_ack:
                print(f"< {line}")
            return line

        handle_background_line(line, stall_samples if stall_samples is not None else [], print_background)


def send_control_command(ser, command, timeout):
    send_command(ser, command, verbose=False)
    line = read_ack(ser, command, timeout, print_background=False, print_ack=False)
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


def probe_gripper(port, baudrate, timeout):
    ser = None
    try:
        ser = open_serial(port, baudrate, timeout)
        send_command(ser, "PING", verbose=False)
        line = read_line(ser, timeout)
        return line == "OK PING GRIPPER"
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


def run_command(ser, command, timeout, wait_for_event, stall_samples):
    send_command(ser, command)

    line = read_ack(ser, command, timeout, stall_samples)
    if line is None:
        return 1
    if not line.startswith("OK "):
        return 1

    if command in {"OPEN", "CLOSE"}:
        print("Waiting for movement event...")
        while True:
            event = read_line(ser, timeout)
            if event is None:
                print(f"Timed out waiting for movement event from {command}", file=sys.stderr)
                return 1

            if event.startswith("EVENT CURRENT "):
                continue

            stall_value = parse_stall_value(event)
            if stall_value is not None:
                stall_samples.append(stall_value)
                continue

            print(f"< {event}")
            if is_move_done_event(event):
                return 0
            if not wait_for_event:
                continue

    return 0


def print_stall_data(stall_samples):
    if not stall_samples:
        print("No stall data collected.")
        return

    for index, sample in enumerate(stall_samples, start=1):
        print(f"{index:04d} {sample['mode']} {sample['value']}")


def listen_for_messages(ser, stall_samples):
    print("Listening for gripper messages. Press Ctrl+C to stop.")
    try:
        while True:
            line = read_line(ser, 0.5)
            if line is not None:
                handle_background_line(line, stall_samples)
    except KeyboardInterrupt:
        print()
        return 0


def interactive_loop(ser, timeout, wait_for_event, stall_samples):
    print("Connected.")
    print("Commands: ping, open, close, stop, status, statistics, listen, data, clear, quit")

    while True:
        try:
            command = input("gripper> ").strip().lower()
        except EOFError:
            print()
            return 0

        if command in {"quit", "exit", "q"}:
            return 0
        if command == "":
            continue
        if command == "listen":
            listen_for_messages(ser, stall_samples)
            continue
        if command == "data":
            print_stall_data(stall_samples)
            continue
        if command == "clear":
            stall_samples.clear()
            print("Stall data cleared.")
            continue
        if command not in COMMANDS:
            print("Unknown command.")
            print("Use: ping, open, close, stop, status, statistics, listen, data, clear, quit")
            continue

        run_command(ser, COMMANDS[command], timeout, wait_for_event, stall_samples)


def main():
    parser = argparse.ArgumentParser(description="Windows USB serial test tool for the Pico gripper.")
    parser.add_argument("command", nargs="?", choices=[*COMMANDS.keys(), "listen", "data", "clear"], help="Command to send.")
    parser.add_argument("--port", help="COM port, for example COM5. If omitted, the script scans for the gripper.")
    parser.add_argument("--baud", type=int, default=BAUDRATE, help=f"Serial baud rate. Default: {BAUDRATE}")
    parser.add_argument("--timeout", type=float, default=TIMEOUT_SECONDS, help="Read timeout in seconds.")
    parser.add_argument(
        "--wait-for-event",
        action="store_true",
        help="After open/close, wait for EVENT MOVE_DONE or EVENT ERROR.",
    )
    args = parser.parse_args()

    port = args.port or auto_detect_port(args.baud, args.timeout)
    if not port:
        print("Could not find gripper USB serial port.", file=sys.stderr)
        print("Try passing it manually, for example: py gripper_usb_test.py --port COM5 ping", file=sys.stderr)
        return 1

    print(f"Using port {port} at {args.baud} baud")

    ser = None
    stall_samples = []
    try:
        ser = open_serial(port, args.baud, args.timeout)
        apply_test_telemetry(ser, args.timeout)

        if args.command is None:
            return interactive_loop(ser, args.timeout, args.wait_for_event, stall_samples)
        if args.command == "listen":
            return listen_for_messages(ser, stall_samples)
        if args.command == "data":
            print_stall_data(stall_samples)
            return 0
        if args.command == "clear":
            stall_samples.clear()
            print("Stall data cleared.")
            return 0

        result = run_command(ser, COMMANDS[args.command], args.timeout, args.wait_for_event, stall_samples)
        if args.command in {"open", "close"}:
            print_stall_data(stall_samples)
        return result
    except (OSError, serial.SerialException) as exc:
        print(f"Serial error: {exc}", file=sys.stderr)
        return 1
    finally:
        if ser is not None:
            restore_default_telemetry(ser, args.timeout)
            ser.close()


if __name__ == "__main__":
    raise SystemExit(main())
