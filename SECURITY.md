# 🛡️ Security Policy

**Gorgeous Core** provides Unreal Engine runtime and editor systems, processes project assets and configuration, and integrates native C++ modules and Blueprint workflows. Security issues in these workflows deserve careful, private handling.

> [!IMPORTANT]
> **Please do not disclose vulnerabilities in public issues, discussions, or comments.** Report them privately to **[vulnerabilities@epicnova.net](mailto:vulnerabilities@epicnova.net)**. Avoid sending credentials, signing keys, or private project files unless a secure exchange method has been agreed upon.

---

## 📦 Supported Versions

Security fixes are evaluated for the **currently maintained codebase**. Older builds and independently modified forks may not receive updates.

| Version / channel | Security support |
| --- | --- |
| Current maintained development branch | Reviewed for security fixes |
| Current published plugin builds | Evaluated case by case |
| Older releases and third-party forks | No guaranteed security maintenance |

> [!NOTE]
> This table describes the maintenance policy, not a guarantee of response time or a statement that every published build has been independently security-audited.

## 📢 Reporting a Vulnerability

Send a private report to **[vulnerabilities@epicnova.net](mailto:vulnerabilities@epicnova.net)**.

To help maintainers reproduce and assess the issue, include:

1. **Affected version:** Release, commit SHA, operating system, and relevant Unreal Engine version.
2. **Attack surface:** Runtime systems, editor modules, Blueprint interfaces, replication, serialization, asset processing, plugin dependencies, or native build integration.
3. **Reproduction:** Minimal steps and a harmless proof of concept, if possible.
4. **Impact:** What an attacker could read, overwrite, execute, or otherwise influence.
5. **Mitigations:** Any known workaround or suggested fix.

Please use synthetic sample projects and redact private paths, API tokens, credentials, and signing material. If sensitive evidence is essential, ask the maintainers to arrange a secure transfer first.

### 🔐 Coordinated disclosure

- We will review incoming reports and work toward reproducing the issue.
- Maintainers may request clarification or additional evidence.
- Fixes and disclosure timing depend on severity, reproducibility, and affected versions.
- Please coordinate public disclosure with the maintainers so affected users have a reasonable opportunity to update.

**No fixed response or remediation SLA is promised.** We will not claim that an issue is resolved until a fix has been verified.

---

## 🔍 Security-Sensitive Areas

| Area | What to watch for |
| --- | --- |
| 🧠 **Runtime data handling** | Unsafe deserialization, invalid object references, and unexpected state mutation |
| 🌐 **Networking and replication** | Untrusted client input, authority checks, RPC validation, and unintended data exposure |
| 🧩 **Blueprint and reflection APIs** | Unsafe callable interfaces, type mismatches, and unexpected execution paths |
| 🛠️ **Editor tooling** | Asset modification, unsafe path handling, and untrusted project data |
| ⚙️ **Native C++ modules** | Memory safety, module loading, dependencies, and build integration |
| 📦 **Distribution and updates** | Artifact authenticity, dependency integrity, and version compatibility |

> [!WARNING]
> Plugin binaries and third-party dependencies should come from trusted sources. A checksum only detects changes relative to a trusted manifest; it does not authenticate the publisher.

## 🧰 Guidance for Users

- Download plugins from official Epic Nova or Gorgeous Things distribution channels.
- Review the origin and contents of third-party plugins before installing them.
- Back up important Unreal Engine projects before applying updates.
- Keep Unreal Engine, development toolchains, and plugin builds up to date.
- Do not share logs containing secrets, access tokens, or sensitive local paths.

## 🤝 Scope and Related Policies

This document covers **technical vulnerabilities in Gorgeous Core**. For harassment or community conduct concerns, use the reporting channel in the [Code of Conduct](CODE_OF_CONDUCT.md), not the security vulnerability inbox.

Additional guidance:

- 📘 [Contributor guidelines](.github/CONTRIBUTING.md)
- 👨‍💻 [Development guide](.github/DEVELOPMENT.md)
- 🧾 [Epic Nova Community Ethic Guideline](https://epicnova.net/legal/community-guidelines/long)
- 🔗 [Epic Nova Network Rules](https://epicnova.net/legal/network-rules/long)

---

## 🙌 Thank You

Responsible vulnerability reporting helps protect developers, projects, and the wider **Gorgeous Things** ecosystem. Thank you for helping keep the plugin ecosystem trustworthy.
