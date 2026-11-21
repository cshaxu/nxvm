"""Restore an owner-supplied OmniBook 300 C-drive ZIP to a raw ATA image."""

import argparse
import re
import stat
import struct
from pathlib import Path
from zipfile import BadZipFile, ZipFile


SECTOR_BYTES = 512
CYLINDERS = 32
HEADS = 16
SECTORS_PER_TRACK = 63
DISK_SECTORS = CYLINDERS * HEADS * SECTORS_PER_TRACK
PARTITION_START = SECTORS_PER_TRACK
FAT_SECTORS = 127
ROOT_ENTRIES = 512
ENTRY_BYTES = 32
DATA_START = PARTITION_START + 1 + 2 * FAT_SECTORS + ROOT_ENTRIES * ENTRY_BYTES // SECTOR_BYTES
DOS_NAME = re.compile(r"[A-Z0-9_~!#$%&'()@^{}-]{1,8}(\.[A-Z0-9_~!#$%&'()@^{}-]{1,3})?", re.ASCII)


def dos_entry(name, attributes, cluster, size, timestamp):
    if name in (".", ".."):
        base, extension = name, ""
    else:
        base, _, extension = name.partition(".")
    year, month, day, hour, minute, second = timestamp
    date = (year - 1980) << 9 | month << 5 | day
    time = hour << 11 | minute << 5 | second // 2
    entry = bytearray(ENTRY_BYTES)
    entry[:11] = f"{base:8}{extension:3}".encode("ascii")
    entry[11] = attributes
    struct.pack_into("<HHHI", entry, 22, time, date, cluster, size)
    return entry


def restore_disk(archive):
    with ZipFile(archive) as backup:
        records = {}
        directories = {(): None}
        for member in backup.infolist():
            path = tuple(member.filename.removesuffix("/").upper().split("/"))
            if not member.filename.isascii() or not all(DOS_NAME.fullmatch(part) for part in path):
                raise ValueError(f"Expected a DOS 8.3 path: {member.filename}")
            mode = member.external_attr >> 16
            if stat.S_ISLNK(mode) or path in records:
                raise ValueError(f"Unsupported or duplicate entry: {member.filename}")
            records[path] = member
            for level in range(1, len(path)):
                directories.setdefault(path[:level], None)
            if member.is_dir():
                directories[path] = member

        files = {path: member for path, member in records.items() if not member.is_dir()}
        if files.keys() & directories.keys():
            raise ValueError("An archive path is both a file and a directory")
        if not {("CONFIG.SYS",), ("AUTOEXEC.BAT",)} <= files.keys():
            raise ValueError("Expected CONFIG.SYS and AUTOEXEC.BAT in the backup root")

        children = {path: [] for path in directories}
        for path in sorted((directories.keys() | files.keys()) - {()}):
            children[path[:-1]].append(path)
        if len(children[()]) > ROOT_ENTRIES:
            raise ValueError("Backup exceeds the FAT16 root directory")

        image = bytearray(DISK_SECTORS * SECTOR_BYTES)
        image[446:454] = bytes((0x80, 1, 1, 0, 0x06, HEADS - 1, SECTORS_PER_TRACK, CYLINDERS - 1))
        struct.pack_into("<II", image, 454, PARTITION_START, DISK_SECTORS - PARTITION_START)
        image[510:512] = b"\x55\xaa"
        boot = PARTITION_START * SECTOR_BYTES
        image[boot:boot + 11] = b"\xeb\x3c\x90MSDOS5.0"
        struct.pack_into("<HBHBHHBHHHII", image, boot + 11,
                         SECTOR_BYTES, 1, 1, 2, ROOT_ENTRIES, DISK_SECTORS - PARTITION_START,
                         0xF8, FAT_SECTORS, SECTORS_PER_TRACK, HEADS, PARTITION_START, 0)
        image[boot + 36:boot + 62] = b"\x80\x00\x29OB30MYOB300    FAT16   "
        image[boot + 510:boot + 512] = b"\x55\xaa"

        fat = [0xFFF8, 0xFFFF]
        first_clusters = {(): 0}
        sizes = {path: (len(entries) + 2) * ENTRY_BYTES
                 for path, entries in children.items() if path}
        sizes.update((path, member.file_size) for path, member in files.items())
        for path, size in sizes.items():
            count = (size + SECTOR_BYTES - 1) // SECTOR_BYTES
            if len(fat) + count - 2 > DISK_SECTORS - DATA_START:
                raise ValueError("Backup exceeds the FAT16 data area")
            first_clusters[path] = len(fat) if count else 0
            if count:
                fat.extend(range(len(fat) + 1, len(fat) + count))
                fat.append(0xFFFF)

        for index in range(2):
            struct.pack_into(f"<{len(fat)}H", image, boot + (1 + index * FAT_SECTORS) * SECTOR_BYTES, *fat)
        for path, entries in children.items():
            if path:
                start = (DATA_START + first_clusters[path] - 2) * SECTOR_BYTES
                stamp = directories[path].date_time if directories[path] else (1980, 1, 1, 0, 0, 0)
                encoded = dos_entry(".", 0x10, first_clusters[path], 0, stamp)
                encoded += dos_entry("..", 0x10, first_clusters[path[:-1]], 0, stamp)
            else:
                start = boot + (1 + 2 * FAT_SECTORS) * SECTOR_BYTES
                encoded = bytearray()
            for child in entries:
                member = records.get(child)
                stamp = member.date_time if member else (1980, 1, 1, 0, 0, 0)
                attributes = member.external_attr & 0x27 if member else 0
                encoded += dos_entry(child[-1], attributes | (0x10 if child in directories else 0x20),
                                     first_clusters[child], files[child].file_size if child in files else 0, stamp)
            image[start:start + len(encoded)] = encoded
        for path, member in files.items():
            if member.file_size:
                start = (DATA_START + first_clusters[path] - 2) * SECTOR_BYTES
                image[start:start + member.file_size] = backup.read(member)
        return image


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("backup", type=Path)
    parser.add_argument("image", type=Path, help="new image path outside the repository")
    args = parser.parse_args()
    try:
        disk = restore_disk(args.backup)
        with args.image.open("xb") as output:
            output.write(disk)
    except (BadZipFile, OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")


if __name__ == "__main__":
    main()
