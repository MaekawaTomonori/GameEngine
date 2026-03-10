# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

---

# Purpose

This project is a modular C++20 DirectX12-based game engine framework.

It is currently built as a static library used by the Game project.

Dependency direction:
Game → Engine

Engine must remain independent from Game.

The architecture should allow a future transition where the Engine becomes the entry point of the application.

The goal of the engine is to provide a safe, simple, and maintainable framework for game development.

---

# Dependency Rules (Critical)

- Engine must never depend on Game.
- Engine must not include Game headers.
- Engine must not assume Game-specific logic.
- All communication must occur through public Engine APIs.
- Game may depend on Engine, but Engine must remain completely standalone.

---

# Architectural Boundaries

- Internal subsystems must remain encapsulated.
- Public APIs must remain minimal and stable.
- Internal implementation details must not leak outside the Engine.
- Low-level graphics objects must remain internal whenever possible.

Engine must not expose raw DirectX12 objects (e.g., `ID3D12Device`, `ID3D12Resource`) through public APIs unless explicitly required.

The Engine should act as an abstraction layer between the Game and the underlying graphics system.

---

# Core Design Principles

The following principles guide all design and implementation decisions.

## Simplicity (KISS)

Prefer simple and clear solutions.

Avoid unnecessary abstraction, complexity, or clever code.

Readable and understandable code is preferred over technically sophisticated solutions.

---

## Avoid Unnecessary Features (YAGNI)

Do not implement features until they are actually required.

Avoid speculative systems or overly generic frameworks.

---

## Don't Repeat Yourself (DRY)

Avoid duplicated logic.

Shared behavior should be centralized and reused when appropriate.

However, readability and simplicity take priority over excessive abstraction.

---

## Single Responsibility (SRP)

Each class or module should have a single, well-defined responsibility.

Large classes should be decomposed into smaller focused components.

---

## Separation of Concerns (SoC)

Different systems should have clearly separated responsibilities.

Examples include:

- Rendering
- Resource management
- Scene management
- Input handling
- Debug utilities

These systems should remain loosely coupled.

---

## Defensive Programming

The engine must remain stable even when users make mistakes.

Examples:

- If a texture fails to load, a default texture should be used.
- If invalid data is passed, the system should fail safely.
- Systems should validate assumptions when possible.

The engine should prioritize safety and stability over strict correctness.

---

## Meaningful Naming

Names are extremely important.

Variables, functions, classes, and systems must clearly express their purpose.

Avoid unclear abbreviations or ambiguous names.

Code should be understandable primarily through good naming.

---

## Boy Scout Rule

When modifying code:

Leave the code cleaner than you found it.

Small improvements such as clearer naming, removing dead code, or improving structure are encouraged if they do not introduce risk.

---

## Avoid Premature Optimization

Premature optimization is the root of many problems.

Do not introduce performance complexity unless there is a measured bottleneck.

Performance optimizations must be based on profiling or clear evidence.

---

# Error Handling Policy

- Exceptions are not used.
- Errors should be returned through return values or status flags.
- Critical failures should provide clear diagnostics.
- The engine should fail safely whenever possible.

---

# AI Behavior Guidelines

When modifying this repository:

- Do not refactor unrelated systems.
- Do not redesign architecture unless explicitly instructed.
- Preserve existing patterns and conventions.
- Ask before introducing structural changes.
- Prefer small, incremental improvements.
- Avoid speculative changes or unnecessary systems.

Follow the Boy Scout Rule when safe.

---

# Priority Order

When making decisions, follow this priority:

1. Maintain architectural integrity
2. Preserve dependency direction
3. Maintain abstraction boundaries
4. Ensure correctness and safety
5. Keep implementations simple
6. Follow coding style rules
7. Optimize performance (only if requested)

---

# Coding Rules

Follow the rules defined in:

- @docs/Coderule.md
- @docs/design-philosophy.md

---

# Build / Execution

Do not attempt to build or run the project.

The user is responsible for compilation and validation.