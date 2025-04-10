---
name: Bug Report
about: File a bug report for Gorgeous Core.
description: File a bug report for Gorgeous Core.
title: "[Bug]: "
labels: ["#01.1 Triage", "#05 Bug Fix"]
assignees:
  - Epgenix
body:
  - type: markdown
    attributes:
      value: |
        🐛 **Bug Report**  
        Thanks for taking the time to report a bug in **Gorgeous Core**! Please provide as much detail as possible to help us fix the issue quickly.

  - type: input
    id: contact
    attributes:
      label: Contact Details
      description: How can we contact you if we need more info?
      placeholder: ex. email@example.com or @yourhandle
    validations:
      required: false

  - type: textarea
    id: what-happened
    attributes:
      label: What happened?
      description: What did you see, and what did you expect to happen?
      placeholder: ex. Crash when spawning a procedural mesh with an invalid spline input
      value: "A bug happened!"
    validations:
      required: true

  - type: textarea
    id: steps
    attributes:
      label: Steps to Reproduce
      description: How can we reproduce the bug?
      placeholder: |
        1. Go to ...
        2. Click on ...
        3. Use this input ...
        4. See error
    validations:
      required: true

  - type: input
    id: version
    attributes:
      label: Plugin Version
      description: What version of Gorgeous Core are you using?
      placeholder: ex. 1.1.0-dev or commit SHA
    validations:
      required: true

  - type: textarea
    id: environment
    attributes:
      label: Environment
      description: Unreal Engine version, OS, and any other relevant setup info.
      placeholder: ex. UE 5.3.2, Windows 11, VS2022
    validations:
      required: true

  - type: textarea
    id: logs
    attributes:
      label: Relevant log output
      description: Paste any relevant output or error messages.
      render: shell
    validations:
      required: false

  - type: checkboxes
    id: terms
    attributes:
      label: Code of Conduct
      description: By submitting this issue, you agree to follow our [Code of Conduct](./CODE_OF_CONDUCT.md).
      options:
        - label: I agree to follow this project's Code of Conduct
          required: true
---

<!-- BUG REPORT TEMPLATE -->

# 🐞 Bug Report – Gorgeous Core

Thanks for taking the time to report a bug in **Gorgeous Core**!  
Please fill out the details below so we can resolve the issue quickly.

---

## 📋 What happened?

_A clear and concise description of the bug._

**Expected:**  
What did you expect to happen?

**Actual:**  
What actually happened?

---

## 🔁 Steps to Reproduce

How can we reproduce the bug?

1. Go to '...'
2. Do this '...'
3. Observe '...'

---

## 🔧 Plugin Version

> ex. `1.1.0-dev`, or commit SHA

---

## 🖥️ Environment

- **Unreal Engine Version**: ex. `5.3.2`
- **Operating System**: ex. `Windows 11`
- **Compiler / IDE**: ex. `Visual Studio 2022`

---

## 🧾 Log Output

```shell
Paste relevant logs, callstacks, or output here
```

---

## 📬 Contact Info (Optional)
How can we reach you if we need more details?

## 🙌 _Thanks for helping improve Gorgeous Core!_

> [!NOTE]
> Check out our [CONTRIBUTING.md](./CONTRIBUTING.md) for info on how to get involved.

> [!WARNING]
> By submitting this issue, you agree to follow our [Code of Conduct](./CODE_OF_CONDUCT.md).
