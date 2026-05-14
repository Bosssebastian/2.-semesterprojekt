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


def send_command(ser, command):
    package = f"CMD {command}\n"
    print(f"> {package.rstrip()}")
    ser.write(package.encode("utf-8"))
    ser.flush()


def probe_gripper(port, baudrate, timeout):
    ser = None
    try:
        ser = open_serial(port, baudrate, timeout)
        send_command(ser, "PING")
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


def run_command(ser, command, timeout, wait_for_event):
    send_command(ser, command)

    line = read_line(ser, timeout)
    if line is None:
        print(f"Timed out waiting for ACK from {command}", file=sys.stderr)
        return 1

    print(f"< {line}")
    if not line.startswith("OK "):
        return 1

    if wait_for_event and command in {"OPEN", "CLOSE"}:
        print("Waiting for movement event...")
        event = read_line(ser, timeout)
        if event is None:
            print(f"Timed out waiting for movement event from {command}", file=sys.stderr)
            return 1
        print(f"< {event}")
        return 0 if event.startswith("EVENT MOVE_DONE ") else 1

    return 0


def listen_for_messages(ser):
    print("Listening for gripper messages. Press Ctrl+C to stop.")
    try:
        while True:
            line = read_line(ser, 0.5)
            if line is not None:
                print(f"< {line}")
    except KeyboardInterrupt:
        print()
        return 0


def interactive_loop(ser, timeout, wait_for_event):
    print("Connected.")
    print("Commands: ping, open, close, stop, status, statistics, listen, quit")

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
            listen_for_messages(ser)
            continue
        if command not in COMMANDS:
            print("Unknown command.")
            print("Use: ping, open, close, stop, status, statistics, listen, quit")
            continue

        run_command(ser, COMMANDS[command], timeout, wait_for_event)


def main():
    parser = argparse.ArgumentParser(description="Windows USB serial test tool for the Pico gripper.")
    parser.add_argument("command", nargs="?", choices=[*COMMANDS.keys(), "listen"], help="Command to send.")
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
    try:
        ser = open_serial(port, args.baud, args.timeout)

        if args.command is None:
            return interactive_loop(ser, args.timeout, args.wait_for_event)
        if args.command == "listen":
            return listen_for_messages(ser)

        return run_command(ser, COMMANDS[args.command], args.timeout, args.wait_for_event)
    except (OSError, serial.SerialException) as exc:
        print(f"Serial error: {exc}", file=sys.stderr)
        return 1
    finally:
        if ser is not None:
            ser.close()


if __name__ == "__main__":
    raise SystemExit(main())
