#!/usr/bin/env python3
"""Echo complete UART lines back to the sender on a Raspberry Pi 5."""

import argparse
import os
import select
import signal
import sys
import termios


BAUD_RATES = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
}


def configure_uart(fd: int, baudrate: int) -> None:
    if baudrate not in BAUD_RATES:
        supported = ", ".join(str(rate) for rate in sorted(BAUD_RATES))
        raise ValueError(f"Unsupported baudrate {baudrate}. Supported: {supported}")

    attrs = termios.tcgetattr(fd)

    # iflag, oflag, cflag, lflag, ispeed, ospeed, cc
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    attrs[3] = 0
    attrs[4] = BAUD_RATES[baudrate]
    attrs[5] = BAUD_RATES[baudrate]
    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 0

    if hasattr(termios, "CRTSCTS"):
        attrs[2] &= ~termios.CRTSCTS

    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)


def main() -> int:
    parser = argparse.ArgumentParser(description="Read UART lines and echo them back.")
    parser.add_argument("--device", default="/dev/serial0", help="UART device path")
    parser.add_argument("--baud", type=int, default=115200, help="UART baudrate")
    args = parser.parse_args()

    running = True

    def stop(_signum, _frame):
        nonlocal running
        running = False

    signal.signal(signal.SIGINT, stop)
    signal.signal(signal.SIGTERM, stop)

    try:
        fd = os.open(args.device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    except OSError as exc:
        print(f"Could not open {args.device}: {exc}", file=sys.stderr)
        return 1

    try:
        configure_uart(fd, args.baud)
    except (OSError, ValueError) as exc:
        os.close(fd)
        print(f"Could not configure UART: {exc}", file=sys.stderr)
        return 1

    print(f"UART echo started on {args.device} at {args.baud} baud")
    print("Send a line ending with newline. Press Ctrl+C to stop.")

    rx_buffer = bytearray()

    try:
        while running:
            readable, _, _ = select.select([fd], [], [], 0.1)
            if not readable:
                continue

            chunk = os.read(fd, 256)
            if not chunk:
                continue

            for byte in chunk:
                if byte == ord("\r"):
                    continue

                if byte == ord("\n"):
                    message = bytes(rx_buffer)
                    rx_buffer.clear()

                    os.write(fd, message + b"\n")
                    print(f"RX/TX: {message.decode(errors='replace')}")
                    continue

                rx_buffer.append(byte)
    finally:
        os.close(fd)
        print("\nUART echo stopped")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
