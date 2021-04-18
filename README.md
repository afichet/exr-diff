# EXR Diff

This is a very simple tool to generate a Delta E 2000 image difference between two EXR files.

It is meant to be compiled easily without any headache on any platform.

It is **not** meant to be fancy or bloated.

## Compilation
Clone the repository:
```bash
git clone https://github.com/afichet/exr-diff.git
cd exr-diff
```

After cloning the repository, ensure you have the submodule:
```bash
git submodule init
git submodule update
```

Then, you're ready to compile
```bash
mkdir build
cd build
cmake ..
make
```

You then can execute the program placed in `bin/`:
```bash
./bin/diff-exr
```

## Usage

### Basic usage:
This compute the Delta E 2000 between two EXR image, with the scale maxing to 10, no exposure compensation.

```bash
diff-exr <exr_image_1> <exr_image_2> -o <image_diff_png>
```

### Options

To see all available options, use `-h` without extra arguments.

## License

This tool uses the following Open Source libraries:
- TinyEXR https://github.com/syoyo/tinyexr
- LodePNG https://lodev.org/lodepng/
- TCLAP http://tclap.sourceforge.net/

It is licensed under the 3-clause BSD license.

