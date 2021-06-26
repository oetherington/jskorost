# JSkorost (ДжайСкорость)

JSkorost is a single-header JSON parser, building and printer written in C99
with a focus on speed and conformability.

## Building

Simply include jskorost.h in your project and define JSKOROST_IMPLEMENTATION in
ONE C or C++ source file.

In order to build the tests you will need to have [cmocka](https://cmocka.org/)
installed.

## Contributing

Contributions are welcome - just send a pull request. Please follow the Linux
kernel style guide and make sure your code compiles without warnings in Clang
and GCC with `-W -Wall -Wextra -pedantic`.

## License

JSkorost is free software under the GNU AGPLv3. See the LICENSE file for
details.
