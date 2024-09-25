import argparse
import json
from pathlib import Path


def check_firmware_size(board: str) -> int:
    root_build_dir = Path(".build") / board
    build_info_json = root_build_dir / "build_info.json"
    build_info = json.loads(build_info_json.read_text())
    assert build_info.keys(), f"No boards found in {root_build_dir}"
    board_info = build_info.get(board) or build_info.keys()[0]
    assert board_info, f"Board {board} not found in {build_info_json}"
    prog_path = Path(board_info["prog_path"])
    base_path = prog_path.parent
    suffixes = [".bin", ".hex", ".uf2"]
    firmware: Path
    for suffix in suffixes:
        firmware = base_path / f"firmware{suffix}"
        if firmware.exists():
            break
    else:
        msg = (
            ", ".join([f"firmware{suffix}" for suffix in suffixes])
            + f" not found in {base_path}"
        )
        raise FileNotFoundError(msg)
    return firmware.stat().st_size


def main(board: str):
    try:
        size = check_firmware_size(board)
        print(f"Firmware size for {board}: {size} bytes")
    except FileNotFoundError as e:
        print(f"Error: {e}")
    except json.JSONDecodeError:
        print(f"Error: Unable to parse build_info.json for {board}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Check FastLED firmware size for the specified board."
    )
    parser.add_argument(
        "--board", type=str, required=True, help="Board to check firmware size for"
    )
    args = parser.parse_args()

    main(args.board)
