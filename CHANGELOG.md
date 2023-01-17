# uLib Changelog

All notable changes to uLib will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
uLib adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.2] - 2023-01-17
### Fixed
- Invalid pointer deallocation in `ustrbuf_to_ustring`.
- Division by zero in `urand_range`.
- Uninitialized `read` and `written` out parameters in `ustream` API.
- Leaked `va_list` in `ustream_multi_writef`.

## [0.2.1] - 2022-12-07
### Added
- `uostream_stderr`.
- `ubit_range`, `ubit_overwrite`, `ubit_lshift`, `ubit_rshift`.
- `ustring_assign_buf`, `ustring_copy_buf`, `ustring_wrap_buf`.
- `ustring_index_of_last`, `ustring_find_last`.

### Changed
- Reworked and renamed bitmask API from `UFlags` to `UBit`.
- Slightly optimized `ustrbuf_to_ustring` in case of small strings.

### Fixed
- Out-of-bounds write in `ustrbuf_to_ustring`.

## [0.2.0] - 2022-11-03
### Added
- Support for Arduino boards.
- Random number and string generators (`urand.h`).
- Per-instance hash and equality support for `UHash` (API identified by the `_pi` suffix).
- `uhash_is_map`, `uhash_size`, `uhash_next`.
- `uvec_move`, `uhash_move`.
- `uvec_get_range`, `uvec_get_range_from`, `uvec_get_range_to`.
- `uistream_std`, `uostream_std`, `uostream_null`.
- `USTREAM_ERR` enum value.
- `ULIB_LIBRARY_TYPE` CMake variable.

### Changed
- All `UVec` and `UHash` functions now require the type argument for consistency.
- `uvec_count` and `uhash_count` do not account for NULL anymore.
- `uvec_foreach` and `uhash_foreach` have been reworked, allowing their bodies to be debugged.
- `uvec_shrink` and `uvec_reserve` do not over-allocate anymore.
- Signature of `ustring_deinit` functions.

### Removed
- `uvec_first_index_where`, `uvec_qsort`, `uvec_qsort_range`,
  `uvec_iterate`, `uvec_iterate_reverse`.
- `uhash_foreach_key`, `uhash_foreach_value`.
- `uvec_alloc`, `uvec_free`, `uhset_alloc`, `uhmap_alloc`, `uhash_free`.
- `uostream_to_null`.
- `ULIB_STATIC`, `ULIB_SHARED` and `ULIB_OBJECT` CMake variables.

### Fixed
- Compilation in C++ projects.
- Out-of-bounds read in `uvec_index_of_sorted`.

## [0.1.4] - 2022-04-27
### Added
- `ustring_assign`, `ustring_copy` and `ustring_wrap` initializers.
- `ustring_size`, `ustring_length` and `ustring_data`.
- `ustrbuf_size`, `ustrbuf_length` and `ustrbuf_data`.
- `ustring_deinit_return_data`.
- `uistream_from_strbuf`, `uistream_from_string` and `uistream_from_ustring`.
- `uostream_to_multi` and `uostream_add_substream`.

### Changed
- Reworked the internals of `UString`.
- Renamed `UVec` and `UString` fields to better signal that they are private.
- Renamed `uvec_reserve_capacity` to `uvec_reserve`.
- Renamed `uvec_storage` to `uvec_data`.
- Renamed `uvec_capacity` to `uvec_size`.
- Renamed `ustring_copy` to `ustring_dup`.
- Renamed `ustring_init_literal` to `ustring_copy_literal`.
- Signature of `ustrbuf_deinit`.

### Removed
- `ustring_init`.

### Fixed
- Parentheses in `utime_is_leap_year`.
- `ulib_float_prev` broken for negative values.

## [0.1.3] - 2022-02-09
### Added
- `uvec_decl` and `uhash_decl`.
- `uvec_capacity`.

### Changed
- Renamed `utime_interval_convert_string` to `utime_interval_to_string`.
- Signature of `uvec_storage`, `uvec_get`, `uvec_set`, `uvec_first` and `uvec_last`.
- `UVec` now stores elements inside its storage pointer when possible.

### Removed
- `uvec_struct` and `uhash_struct`.
- `uvec_uint` and `uhash_uint`.

### Fixed
- Potential memory leaks due to missing `ustrbuf_deinit`.
- Docs for `ustring_index_of` and `ustring_find`.

## [0.1.2] - 2022-02-01
### Added
- `uostream_write_string`, `uostream_write_time`,
  `uostream_write_time_interval`, `uostream_write_version`.
- `uversion_to_string`.

### Changed
- Renamed `utime_interval_convert_string` to `utime_interval_to_string`.

### Fixed
- Date and time format for `utime_to_string`.

## [0.1.1] - 2022-01-27
### Added
- Datetime type: `UTime`.
- Builtin UVec types: `UVec(ulib_byte)`, `UVec(ulib_int)`, `UVec(ulib_uint)`,
                      `UVec(ulib_float)`, `UVec(ulib_ptr)`, `UVec(UString)`.
- Builtin UHash types: `UHash(ulib_int)`, `UHash(ulib_uint)`, `UHash(ulib_ptr)`,
                       `UHash(UString)`.

### Changed
- Renamed `utime_unit` enum values.
- Renamed `utime_unit_auto`, `utime_convert` and `utime_convert_string` to
  `utime_interval_unit_auto`, `utime_interval_convert` and `utime_interval_convert_string`.

### Fixed
- Use `FLT_TRUE_MIN` and `DBL_TRUE_MIN` to define `ULIB_FLT_MIN`.

## [0.1.0] - 2021-12-15
### Added
- Numeric types: `ulib_int`, `ulib_uint`, `ulib_float`, `ulib_byte`.
- String types: `UString`, `UStrBuf`.
- IO streams: `UIStream`, `UOStream`.
- Collections: `UHash`, `UVec`.
- Versioning: `UVersion`.
- Bitmask manipulation primitives.
- Custom allocators.
- Time measurement utilities.
- Test utilities.
- Miscellaneous helper macros.

[0.2.2]: https://github.com/ivanobilenchi/ulib/compare/v0.2.1...v0.2.2
[0.2.1]: https://github.com/ivanobilenchi/ulib/compare/v0.2.0...v0.2.1
[0.2.0]: https://github.com/ivanobilenchi/ulib/compare/v0.1.4...v0.2.0
[0.1.4]: https://github.com/ivanobilenchi/ulib/compare/v0.1.3...v0.1.4
[0.1.3]: https://github.com/ivanobilenchi/ulib/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/ivanobilenchi/ulib/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/ivanobilenchi/ulib/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/ivanobilenchi/ulib/releases/tag/v0.1.0
