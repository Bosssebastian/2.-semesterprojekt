#!/usr/bin/env python3
"""Plot event-based stall log CSV files.

Expected CSV format:
    event,move,result,stall_mode,values
    MOVE_DONE,CLOSE,STALL,current,"0.041 0.044 0.052"
    MOVE_DONE,OPEN,STEPS_FINISHED,current,"0.038 0.040"
    MOVE_DONE,OPEN_RESET,STALL,current,"0.151 0.169"
    MOVE_DONE,OPEN_RESET_FORWARD,STEPS_FINISHED,current,"0.047 0.055"

Examples:
    python plot_stall_values.py --list-moves
    python plot_stall_values.py
    python plot_stall_values.py --all
    python plot_stall_values.py --move OPEN
    python plot_stall_values.py --row 3
    python plot_stall_values.py --each-move
    python plot_stall_values.py --each-sequence
"""

from __future__ import annotations

import argparse
import csv
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import matplotlib.pyplot as plt


DEFAULT_CSV = Path(__file__).with_name("stall_values.csv")
DEFAULT_OUTPUT_DIR = Path(__file__).with_name("charts")
EXPECTED_EVENT = "MOVE_DONE"
EXPECTED_RUN_STEPS = (
    ("CLOSE", "STALL"),
    ("OPEN", "STEPS_FINISHED"),
    ("OPEN_RESET", "STALL"),
    ("OPEN_RESET_FORWARD", "STEPS_FINISHED"),
)


@dataclass(frozen=True)
class MoveRecord:
    row_number: int
    event: str
    move: str
    result: str
    stall_mode: str
    values: list[float]

    @property
    def label(self) -> str:
        return f"{self.row_number:03d}_{self.event}_{self.move}_{self.result}_{self.stall_mode}"


@dataclass(frozen=True)
class PlotSegment:
    record: MoveRecord
    start_x: int
    end_x: int


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Make charts from event-based stall CSV logs.")
    parser.add_argument(
        "--csv",
        type=Path,
        default=DEFAULT_CSV,
        help=f"CSV file to read. Default: {DEFAULT_CSV}",
    )
    parser.add_argument(
        "-o",
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help=f"Folder where charts are saved. Default: {DEFAULT_OUTPUT_DIR}",
    )
    parser.add_argument(
        "--list-moves",
        action="store_true",
        help="Print all recorded moves and exit.",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Plot all rows in one continuous chart.",
    )
    parser.add_argument(
        "--move",
        help="Plot all rows where move matches this value, for example OPEN or CLOSE.",
    )
    parser.add_argument(
        "--result",
        help="Only include rows with this result, for example STALL or STEPS_FINISHED.",
    )
    parser.add_argument(
        "--stall-mode",
        help="Only include rows with this stall mode, for example current or uart.",
    )
    parser.add_argument(
        "--row",
        type=int,
        help="Plot one completed move by CSV data row number. The first data row is 1.",
    )
    parser.add_argument(
        "--each-move",
        action="store_true",
        help="Save one chart per CSV row.",
    )
    parser.add_argument(
        "--each-sequence",
        action="store_true",
        help="Save one chart per full four-move auto run.",
    )
    parser.add_argument(
        "--show",
        action="store_true",
        help="Open the chart window after saving.",
    )
    return parser.parse_args()


def read_records(csv_path: Path) -> list[MoveRecord]:
    if not csv_path.exists():
        raise SystemExit(f"CSV file does not exist: {csv_path}")

    records: list[MoveRecord] = []
    with csv_path.open(newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)
        required_columns = {"event", "move", "result", "stall_mode", "values"}
        missing_columns = required_columns.difference(reader.fieldnames or [])
        if missing_columns:
            missing = ", ".join(sorted(missing_columns))
            raise SystemExit(f"CSV is missing required columns: {missing}")

        for row_number, row in enumerate(reader, start=1):
            raw_values = row["values"].strip()
            values = [float(token) for token in raw_values.split()] if raw_values else []
            records.append(
                MoveRecord(
                    row_number=row_number,
                    event=row["event"].strip().upper(),
                    move=row["move"].strip().upper(),
                    result=row["result"].strip().upper(),
                    stall_mode=row["stall_mode"].strip().lower(),
                    values=values,
                )
            )

    if not records:
        raise SystemExit(f"CSV file contains no move rows: {csv_path}")
    return records


def print_moves(records: list[MoveRecord]) -> None:
    event_counts = Counter(record.event for record in records)
    move_counts = Counter(record.move for record in records)
    mode_counts = Counter(record.stall_mode for record in records)
    runs = detect_sequences(records)

    print("Summary:")
    print("  Events: " + ", ".join(f"{event}={count}" for event, count in sorted(event_counts.items())))
    print("  Moves: " + ", ".join(f"{move}={count}" for move, count in sorted(move_counts.items())))
    print("  Modes: " + ", ".join(f"{mode}={count}" for mode, count in sorted(mode_counts.items())))
    print(f"  Runs: {sum(1 for run in runs if is_good_run(run))} good / {len(runs)} total")
    print()
    print("Rows:")
    for record in records:
        print(
            f"  {record.row_number:3d}  {record.event:9}  {record.move:18}  {record.result:14}  "
            f"{record.stall_mode:7}  {len(record.values)} samples"
        )


def detect_sequences(records: list[MoveRecord]) -> list[list[MoveRecord]]:
    sequences: list[list[MoveRecord]] = []
    current_sequence: list[MoveRecord] = []
    expected_index = 0

    for record in records:
        if record.move == EXPECTED_RUN_STEPS[0][0] and current_sequence:
            sequences.append(current_sequence)
            current_sequence = []
            expected_index = 0

        expected_move = EXPECTED_RUN_STEPS[expected_index][0]
        if current_sequence and record.move != expected_move:
            sequences.append(current_sequence)
            current_sequence = []
            expected_index = 0

        current_sequence.append(record)
        expected_index += 1

        if expected_index == len(EXPECTED_RUN_STEPS):
            sequences.append(current_sequence)
            current_sequence = []
            expected_index = 0

    if current_sequence:
        sequences.append(current_sequence)

    return sequences


def is_good_run(sequence: list[MoveRecord]) -> bool:
    if len(sequence) != len(EXPECTED_RUN_STEPS):
        return False

    actual_steps = tuple((record.move, record.result) for record in sequence)
    return actual_steps == EXPECTED_RUN_STEPS and all(record.event == EXPECTED_EVENT for record in sequence)


def run_status(sequence: list[MoveRecord]) -> str:
    return "PASS" if is_good_run(sequence) else "FAIL"


def filtered_records(records: list[MoveRecord], args: argparse.Namespace) -> list[MoveRecord]:
    selected = records
    if args.move:
        wanted_move = args.move.strip().upper()
        selected = [record for record in selected if record.move == wanted_move]
    if args.result:
        wanted_result = args.result.strip().upper()
        selected = [record for record in selected if record.result == wanted_result]
    if args.stall_mode:
        wanted_mode = args.stall_mode.strip().lower()
        selected = [record for record in selected if record.stall_mode == wanted_mode]
    if args.row is not None:
        selected = [record for record in selected if record.row_number == args.row]
    if not selected:
        raise SystemExit("No CSV rows matched the selected filters. Use --list-moves to see choices.")
    return selected


def grouped_by_mode(records: Iterable[MoveRecord]) -> dict[str, list[MoveRecord]]:
    groups: dict[str, list[MoveRecord]] = {}
    for record in records:
        groups.setdefault(record.stall_mode, []).append(record)
    return groups


def mode_sort_key(mode: str) -> tuple[int, str]:
    return (0 if mode == "current" else 1 if mode == "uart" else 2, mode)


def configure_axis(ax, mode: str) -> None:
    if mode == "current":
        ax.set_ylabel("Current (A)")
    elif mode == "uart":
        ax.set_ylabel("UART stall value")
    else:
        ax.set_ylabel(f"{mode} value")
    ax.grid(True, alpha=0.25)


def adaptive_figsize(records: list[MoveRecord], mode_count: int) -> tuple[float, float]:
    total_samples = sum(len(record.values) for record in records)
    width = min(max(9.0, total_samples / 45.0), 80.0)
    height = 4.5 + 2.4 * (mode_count - 1)
    return (width, height)


def plot_segments(records: list[MoveRecord]) -> list[PlotSegment]:
    segments: list[PlotSegment] = []
    x_offset = 0
    for record in records:
        end_x = x_offset + max(len(record.values) - 1, 0)
        segments.append(PlotSegment(record=record, start_x=x_offset, end_x=end_x))
        x_offset += len(record.values) + 1
    return segments


def draw_plot(records: list[MoveRecord], title: str, output_path: Path, show: bool) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    segments = plot_segments(records)
    mode_groups = grouped_by_mode(records)
    mode_names = sorted(mode_groups, key=mode_sort_key)
    total_width = max((segment.end_x for segment in segments), default=0)

    fig, axes = plt.subplots(
        len(mode_names),
        1,
        sharex=False,
        figsize=adaptive_figsize(records, len(mode_names)),
        constrained_layout=True,
    )
    if len(mode_names) == 1:
        axes = [axes]

    for ax, mode in zip(axes, mode_names):
        mode_records = mode_groups[mode]
        for segment in segments:
            record = segment.record
            if record.stall_mode != mode:
                continue

            x_values = list(range(segment.start_x, segment.start_x + len(record.values)))
            ax.plot(
                x_values,
                record.values,
                linewidth=1.2,
                label=f"row {record.row_number}: {record.move} {record.result}",
            )

            if len(records) > 1:
                ax.axvline(segment.start_x, color="black", alpha=0.14, linewidth=0.9)
                ax.text(
                    segment.start_x,
                    0.98,
                    f"{record.move} {record.result}",
                    transform=ax.get_xaxis_transform(),
                    rotation=90,
                    va="top",
                    ha="right",
                    fontsize=8,
                    alpha=0.65,
                )

        configure_axis(ax, mode)
        ax.set_xlabel("Sample index")
        ax.set_xlim(left=0, right=max(total_width, 1))
        if len(mode_records) <= 20:
            ax.legend(loc="upper right", fontsize=8)

    fig.suptitle(title)
    fig.savefig(output_path, dpi=160)
    print(f"Saved {output_path}")

    if show:
        plt.show()
    else:
        plt.close(fig)


def safe_filename(name: str) -> str:
    return "".join(char if char.isalnum() or char in "-_." else "_" for char in name)


def describe_selection(records: list[MoveRecord]) -> str:
    if len(records) == 1:
        record = records[0]
        return f"row {record.row_number}: {record.event} {record.move} {record.result} {record.stall_mode}"

    moves = "/".join(sorted({record.move for record in records}))
    modes = "/".join(sorted({record.stall_mode for record in records}))
    return f"{len(records)} moves ({moves}, {modes})"


def sequence_label(sequence_index: int, sequence: list[MoveRecord]) -> str:
    first_row = sequence[0].row_number
    last_row = sequence[-1].row_number
    moves = "-".join(record.move for record in sequence)
    return f"run_{sequence_index:03d}_{run_status(sequence)}_rows_{first_row:03d}-{last_row:03d}_{moves}"


def generate_full_set(records: list[MoveRecord], output_dir: Path, show: bool) -> None:
    draw_plot(
        records,
        "All stall value samples",
        output_dir / "full_file" / "stall_values_full_file.png",
        show,
    )

    for sequence_index, sequence in enumerate(detect_sequences(records), start=1):
        draw_plot(
            sequence,
            f"Auto run {sequence_index} {run_status(sequence)}: rows {sequence[0].row_number}-{sequence[-1].row_number}",
            output_dir / "full_sequences" / f"stall_values_{safe_filename(sequence_label(sequence_index, sequence))}.png",
            show,
        )

    for record in records:
        draw_plot(
            [record],
            f"Stall samples for {describe_selection([record])}",
            output_dir / "moves" / f"stall_values_{safe_filename(record.label)}.png",
            show,
        )


def main() -> None:
    args = parse_args()
    records = read_records(args.csv)

    if args.list_moves:
        print_moves(records)
        return

    chart_mode_selected = (
        args.all
        or args.move
        or args.result
        or args.stall_mode
        or args.row is not None
        or args.each_move
        or args.each_sequence
    )
    if not chart_mode_selected:
        generate_full_set(records, args.output_dir, args.show)
        return

    if args.each_move:
        for record in filtered_records(records, args):
            draw_plot(
                [record],
                f"Stall samples for {describe_selection([record])}",
                args.output_dir / f"stall_values_{safe_filename(record.label)}.png",
                args.show,
            )
        return

    if args.each_sequence:
        selected_records = filtered_records(records, args)
        for sequence_index, sequence in enumerate(detect_sequences(selected_records), start=1):
            draw_plot(
                sequence,
                f"Auto run {sequence_index} {run_status(sequence)}: rows {sequence[0].row_number}-{sequence[-1].row_number}",
                args.output_dir / "full_sequences" / f"stall_values_{safe_filename(sequence_label(sequence_index, sequence))}.png",
                args.show,
            )
        return

    selected = filtered_records(records, args)
    title = "All stall value samples" if args.all else f"Stall samples for {describe_selection(selected)}"

    output_name = "stall_values_all.png"
    if chart_mode_selected and not args.all:
        parts = ["stall_values"]
        if args.move:
            parts.append(args.move.strip().upper())
        if args.result:
            parts.append(args.result.strip().upper())
        if args.stall_mode:
            parts.append(args.stall_mode.strip().lower())
        if args.row is not None:
            parts.append(f"row_{args.row}")
        output_name = safe_filename("_".join(parts)) + ".png"

    draw_plot(selected, title, args.output_dir / output_name, args.show)


if __name__ == "__main__":
    main()
