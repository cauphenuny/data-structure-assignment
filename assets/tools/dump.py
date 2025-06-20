import sys

import cxxfilt  # For demangling C++ symbols
import lief  # For parsing object files


def dump_symbols(objfile: str):
    objects = list[tuple[str, str, str]]()
    try:
        binary = lief.parse(objfile)

        if binary is None:
            raise ValueError(f"Could not parse the file: {objfile}")

        print(f"Symbols from {objfile}:")
        print("-" * 60)

        for symbol in binary.symbols:
            address = f"{symbol.value:016x}" if symbol.value else "                "

            if isinstance(symbol.name, bytes):
                raise ValueError(f"Symbol name is bytes, not str: {symbol.name}")

            try:
                demangled_name = cxxfilt.demangle(symbol.name, external_only=False)
                objects.append((address, demangled_name, symbol.name))
            except cxxfilt.InvalidName:
                demangled_name = symbol.name

    except Exception as e:
        print(f"Error processing {objfile}: {e}")
        sys.exit(1)

    objects.sort(key=lambda x: x[1])

    for address, name, original_name in objects:
        print(f"{address} {name} {original_name}")


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <objfile>")
        sys.exit(1)

    dump_symbols(sys.argv[1])


if __name__ == "__main__":
    main()
