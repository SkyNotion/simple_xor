# simple_xor (sxor)

Sometimes you don't need crazy encryption (AES...), you just want to keep a file private. Just xor encode it.

## Building

Windows is not supported because of the use of POSIX headers (e.g `unistd.h`). Although POSIX compatible compiler exists for windows.

NOTE: it also hides the original filename

```bash
git clone https://github.com/SkyNotion/simple_xor.git

cd simple_xor

gcc sxor.c -o sxor
```

## Usage
```bash
Usage: sxor [options] file1.dat file2.txt ...
-e to encode
-d to decode
-k the key to use for the encode or decode (max length: 4096)

# To encode a file
./sxor -e -f file.txt -k 'my_key'

# To decode a file
./sxor -d -f file.dat -k 'my_key'
```