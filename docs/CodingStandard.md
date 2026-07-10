# Coding Standard

## Naming Conventions
- **Classes/Types**: `PascalCase`
- **Methods/Functions**: `camelCase`
- **Variables**: `camelCase`
- **Constants/Macros**: `UPPER_CASE`
- **Member variables**: `m_` prefix
- **Static members**: `s_` prefix
- **Namespaces**: `snake_case` (nested: `core`, `engine`, `game`)

## File Conventions
- Headers: `#pragma once` guard, `.hpp` extension
- Source: `.cpp` extension
- One class per file (exceptions: small utility structs, type aliases)

## Namespaces
- No `using namespace` in headers (except in anonymous namespaces in .cpp files)
- Use fully-qualified names or short aliases in .cpp files

## Formatting
- Enforced by `.clang-format` (Google style, 4-space indent, 120 column limit)
- Run `./scripts/format.sh` before committing

## Error Handling
- No exceptions in release builds
- Use `std::optional` or `std::expected`-style for recoverable errors
- Assert (via assert/Catch2 REQUIRE) for programming errors

## Pointers & Memory
- Prefer `std::unique_ptr` for ownership
- Use `std::shared_ptr` only for shared ownership (e.g., ResourceManager cache)
- Raw pointers for non-owning observers only
- No `new`/`delete` outside of smart pointer wrappers

## Const Correctness
- Mark methods `const` when they don't mutate
- Pass read-only parameters by `const&`
- Use `constexpr` for compile-time-known values

## Testing
- All public API gets unit tests
- Tests follow TDD: Red (failing test) → Green (passing implementation) → Refactor
- Use Catch2 v3 with BDD-style sections
- Test file mirrors source structure: `tests/unit/<module>/test_<file>.cpp`
