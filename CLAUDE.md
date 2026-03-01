# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Purpose

 This project is a modular C++20 DirectX12-based game engine framework.
 It is currently built as a static lib used by the Game project.
 The dependency direction is : Game -> Engine.

 Engine must remain independent from game.
 The architecture should allow future transition where Engine becomes the entry point.

 ## Dependency Rules (Critical)

- Engine must never depend on Game.
- Engine must not include Game headers.
- Engine must not assume Game-specific logic.
- All communication must occur through public Engine APIs.

## Architectural Boundaries

- Internal subsystems must remain encapsulated.
- Public APIs must remain minimal and stable.
- Do not expose internal implementation details.
- Engine must not expose raw DirectX12 objects (e.g., ID3D12Device, ID3D12Resource) through public APIs unless explicitly required.

## Core Design Principles

- Prioritize usability for Engine users.
- Ensure safety even if internal complexity increases.
- Follow SOLID, DRY, KISS, YAGNI principles.
- Avoid exceptions. Use return values for error handling.
- Prefer composition over inheritance.

## AI Behavior Guidelines

- Do not refactor unrelated systems.
- Do not redesign architecture unless explicitly instructed.
- Preserve existing patterns and naming conventions.
- Ask before introducing structural changes.
- Do not optimize unless performance is explicitly requested.

## Priority Order

1. Maintain architectural integrity
2. Preserve dependency direction
3. Maintain abstraction boundaries
4. Ensure correctness and safety
5. Follow coding style rules
6. Optimize performance (only if requested)

## Coding Rules

Follow the rules defined in:
- @docs/Coderule.md
- @docs/design-philosophy.md

## Build / Execution

Do not attempt to build or run the project.
The user will handle compilation and validation.

