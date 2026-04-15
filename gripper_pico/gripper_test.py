#!/usr/bin/env python3
import argparse
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("pyserial is required. Install it with: pip install pyserial", file=sys.stderr)
    raise SystemExit(1)


BAUDRATE = 115200
TIMEOUT = 1.0


def open_serial(port, baudrate, timeout):
    ser = serial.Serial(port=port, baudrate=baudrate, timeout=0.1, write_timeout=timeout)
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


def probe_gripper(port, baudrate, timeout):
    ser = None
    try:
        ser = open_serial(port, baudrate, timeout)
        send_command(ser, "PING")
        line = read_line(ser, timeout)
        return line == "OK PING GRIPPER"
    except OSError:
        return False
    finally:
        if ser is not None:
            ser.close()


def auto_detect_port(baudrate, timeout):
    for port_info in list_ports.comports():
        if probe_gripper(port_info.device, baudrate, timeout):
            return port_info.device
    return None


def send_command(ser, command):
    ser.write(f"CMD {command}\n".encode("utf-8"))


def run_command(ser, command, timeout):
    send_command(ser, command)
    line = read_line(ser, timeout)
    if line is None:
        print(f"Timed out waiting for {command}", file=sys.stderr)
        return 1

    print(line)
    return 0 if line.startswith("OK ") else 1


def listen_for_events(ser):
    print("Listening for gripper messages. Press Ctrl+C to stop.")
    try:
        while True:
            line = read_line(ser, 0.5)
            if line is not None:
                print(line)
    except KeyboardInterrupt:
        print("\nStopped.")


def interactive_loop(ser, timeout):
    print("Connected. Commands: ping, open, close, stop, listen, quit")
    while True:
        try:
            command = input("> ").strip().lower()
        except EOFError:
            print()
            return 0

        if command in {"quit", "exit", "q"}:
            return 0
        if command == "":
            continue
        if command == "listen":
            listen_for_events(ser)
            continue

        mapping = {
            "ping": "PING",
            "open": "OPEN",
            "close": "CLOSE",
            "stop": "STOP",
        }

        if command not in mapping:
            print("Unknown command. Use: ping, open, close, stop, listen, quit")
            continue

        run_command(ser, mapping[command], timeout)


def main():
    parser = argparse.ArgumentParser(description="Simple USB test program for the Pico gripper.")
    parser.add_argument("--port", help="COM port, for example COM5. If omitted, the script auto-detects the gripper.")
    parser.add_argument("--baud", type=int, default=BAUDRATE, help="Serial baud rate")
    parser.add_argument("--timeout", type=float, default=TIMEOUT, help="Read timeout in seconds")
    parser.add_argument(
        "command",
        nargs="?",
        choices=["ping", "open", "close", "stop", "listen"],
        help="Optional single command. If omitted, interactive mode starts.",
    )
    args = parser.parse_args()

    port = args.port or auto_detect_port(args.baud, args.timeout)
    if not port:
        print("Could not find gripper USB serial port.", file=sys.stderr)
        return 1

    print(f"Using port {port}")

    ser = None
    try:
        ser = open_serial(port, args.baud, args.timeout)

        if args.command is None:
            return interactive_loop(ser, args.timeout)
        if args.command == "listen":
            listen_for_events(ser)
            return 0

        return run_command(ser, args.command.upper(), args.timeout)
    except OSError as exc:
        print(f"Failed to open {port}: {exc}", file=sys.stderr)
        return 1
    finally:
        if ser is not None:
            ser.close()


if __name__ == "__main__":
    raise SystemExit(main())
