# EXR Diff

This is a very simple tool to generate a Delta E 2000 image difference between two EXR files using D65 white achromatic reference illuminant (CIE 1931 2°).

It is meant to be compiled easily without any headache on any platform.

It is **not** meant to be fancy or bloated.

**Warning! This tool does not currently support:**

- **Custom chromaticities: chromaticities specified in the EXR header will be ignored. Rec 709 is assumed. If you're using any other colorspace, the resulting Delta E will not be correct.**
- **Luminance chrominance images.**

## Compilation
Clone the repository:
```bash
git clone https://github.com/afichet/exr-diff.git
cd exr-diff
```

After cloning the repository, ensure you have the submodule:
```bash
git submodule update --init --recursive
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

It also uses the colormaps from https://bids.github.io/colormap/ by Nathaniel J. Smith, Stefan van der Walt, and (in the case of viridis) Eric Firing. These are licensed under CC0 license (http://creativecommons.org/publicdomain/zero/1.0/).

It is licensed under the 3-clause BSD license.

