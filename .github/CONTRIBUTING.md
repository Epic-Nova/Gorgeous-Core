# Contributing to Gorgeous Core

First off, thanks for taking the time to contribute! ❤️

All types of contributions are encouraged and valued. See the [Table of Contents](#table-of-contents) for different ways to help and details about how this project handles them. Please make sure to read the relevant section before making your contribution. It will make it a lot easier for us maintainers and smooth out the experience for all involved. The community looks forward to your contributions. 🎉

> And if you like the project, but just don't have time to contribute, that's fine. There are other easy ways to support the project and show your appreciation, which we would also be very happy about:
> - ⭐ **Star the project** on [GitHub](https://github.com/Epic-Nova/Gorgeous-Core)
> - 📖 **Refer this project** in your project's README
> - 👥🥂 **Mention the project** at local meetups and tell your friends/colleagues
> - ☕️ **Download the plugin** from the [Fab Marketplace](<FAB_MARKETPLACE_LINK_HERE>)

## Table of Contents

- [I Have a Question](#-i-have-a-question)
- [I Want To Contribute](#-i-want-to-contribute)
    - [Reporting Bugs](#-reporting-bugs)
    - [Reporting Vulnerabilities](#%EF%B8%8F-reporting-vulnerabilities)
    - [Suggesting Enhancements](#-suggesting-enhancements)
    - [Your First Code Contribution](#-your-first-code-contribution)
    - [Improving The Documentation](#-improving-the-documentation)
- [Styleguides](#-styleguides)
    - [Naming Conventions](#%EF%B8%8F-naming-conventions)
    - [Code Comments](#-code-comments)
    - [Commit Messages](#-commit-messages)
    - [Documentation Pages](#-documentation-pages)
- [Join The Project Team](#-join-the-project-team)

---

## ❓ I Have a Question

> [!NOTE]
> If you want to ask a question, we assume that you have read the available [Documentation](https://c.docs.gt.simsalabim.studio/).

Before you ask a question, it is best to search for existing [Issues](https://github.com/Epic-Nova/Gorgeous-Core/issues) that might help you. In case you have found a suitable issue and still need clarification, you can write your question in this issue. It is also advisable to search the internet for answers first.

If you then still feel the need to ask a question and need clarification, we recommend the following:

- Read the [FAQ.md](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/.github/FAQ.md)
- Open an [Question Issue](https://github.com/Epic-Nova/Gorgeous-Core/issues/new?template=question.md).
- Write an email to [support@epicnova.net](mailto:support@epicnova.net)

We will then take care of the issue as soon as possible.

---

## 🤝 I Want To Contribute

> [!IMPORTANT]  
> When contributing to this project, **you must agree that you have authored 100% of the content**, that you have the necessary rights to the content and that the content you contribute may be provided under the project license.

### 🐞 Reporting Bugs

#### 🐞 Before Submitting a Bug Report

A good bug report shouldn't leave others needing to chase you up for more information. Therefore, we ask you to investigate carefully, collect information and describe the issue in detail in your report. Please complete the following steps in advance to help us fix any potential bug as fast as possible.

- ✅ **Make sure that you are using the latest version.**
- 🤔 **Determine if your bug is really a bug** and not an error on your side, e.g., using incompatible environment components/versions. (Make sure that you have read the [documentation](https://c.docs.gt.simsalabim.studio/). If you are looking for support, you might want to check [this section](#-i-have-a-question)).
- 🔍 **Check the [bug tracker](https://github.com/orgs/Epic-Nova/projects/12/views/5)** to see if a bug report already exists for your bug or error.
- 🌐 **Search the internet** (including Stack Overflow) to see if users outside of the GitHub community have discussed the issue.
- Gather information about the bug:
    - 📜 **Stack trace (Traceback)**
    - 💻 **OS, Platform, and Version** (Windows, Linux, macOS, x86, ARM)
    - ⚙️ **Version of the engine, compiler, SDK, runtime environment.**
    - ➡️ **Possibly your input and the output**
    - 🔁 **Can you reliably reproduce the issue?** And can you also reproduce it with older versions?

#### 📝 How Do I Submit a Good Bug Report?

> [!IMPORTANT]
> 🚨 **You must never report security-related issues, vulnerabilities, or bugs including sensitive information to the issue tracker or elsewhere in public.** How to handle vulnerability reporting is described [here](#%EF%B8%8F-reporting-vulnerabilities).

We use GitHub issues to track bugs and errors. If you run into an issue with the project:

- 🐛 **Open an [Issue](https://github.com/Epic-Nova/Gorgeous-Core/issues/new?template=bug_report.md).**
- ✍️ **Explain the behavior you would expect and the actual behavior.**
- ⚙️ **Please provide as much context as possible** and describe the ***reproduction steps*** that someone else can follow to recreate the issue on their own. This usually includes your code. If the code that you are experiencing issues with is Blueprint, then copy your script code and paste it in [BlueprintUE](https://blueprintue.com/) and share the link in the issue. For good bug reports, you should isolate the problem and create a reduced test case.
- ℹ️ **Provide the information you collected in the previous section.**

Once it's filed:

- The project team will label the issue accordingly.
- A team member will try to reproduce the issue with your provided steps. If there are no reproduction steps or no obvious way to reproduce the issue, the team will ask you for those steps and mark the issue as $${\color{gold}Needs Update}$$. Bugs with the $${\color{gold}Needs Update}$$ tag will not be addressed until they are reproduced.
- If the team is able to reproduce the issue, then the $${\color{steelblue}Triage}$$ tag will be removed to indicate that this is a valid bug report. Also it will be marked possibly with other tags (such as $${\color{darkblue}Noted}$$), and the issue will be left to be [implemented by someone](#-your-first-code-contribution).

---

### 🛡️ Reporting Vulnerabilities

> [!NOTE]
> Please report vulnerabilities responsibly to help us ensure the safety and security of our community and users.

Vulnerabilities should be reported directly to the project maintainers via a secure and private channel. **Do not disclose security-related issues publicly on the issue tracker or any other public platform.**

To report a security vulnerability, please send an email to: [**vulnerabilities@epicnova.net**](mailto:vulnerabilities@epicnova.net)

To ensure the confidentiality of your report, we strongly encourage you to encrypt your communication using our PGP public key. You can find the public key here:

[🔗 **PGP Public Key**](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/.github/SensitiveReportingPublicKey.asc)

By encrypting your report, you can help protect sensitive information during transmission. We appreciate your cooperation in handling security matters responsibly. Our team will acknowledge your report promptly and work diligently to address the issue.

---

### ✨ Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for Gorgeous Core, **including completely new features and minor improvements to existing functionality**. Following these guidelines will help maintainers and the community to understand your suggestion and find related suggestions.

#### 🤔 Before Submitting an Enhancement

- ✅ **Make sure that you are using the latest version.**
- 📖 **Read the [documentation](https://c.docs.gt.simsalabim.studio/) carefully** and find out if the functionality is already covered, maybe by an individual configuration.
- 🔍 **Perform a [search](https://github.com/orgs/Epic-Nova/projects/12/views/4)** to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.
- 💡 **Find out whether your idea fits with the scope and aims of the project.** It's up to you to make a strong case to convince the project's developers of the merits of this feature. Keep in mind that we want features that will be useful to the majority of our users and not just a small subset.

#### 📝 How Do I Submit a Good Enhancement Suggestion?

Enhancement suggestions are tracked as [Feature issues](https://github.com/Epic-Nova/Gorgeous-Core/issues/new?template=feature_request.md).

- 🏷️ **Use a clear and descriptive title** for the issue to identify the suggestion.
- 📝 **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
- ➡️ **Describe the current behavior** and **explain which behavior you expected to see instead** and why. At this point you can also tell which alternatives do not work for you.
- 🖼️ **You may want to include screenshots or screen recordings** which help you demonstrate the steps or point out the part which the suggestion is related to. You can use [LICEcap](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and the built-in [screen recorder in GNOME](https://help.gnome.org/users/gnome-help/stable/screen-shot-record.html.en) or [SimpleScreenRecorder](https://github.com/MaartenBaert/ssr) on Linux. <!-- this should only be included if the project has a GUI -->
- **Explain why this enhancement would be useful** to most Gorgeous Core users. You may also want to point out the other projects that solved it better and which could serve as inspiration.

---

### 🧑‍💻 Your First Code Contribution

We appreciate your interest in contributing code to Gorgeous Core! Here's a quick guide to get you started:

1.  **Fork the Repository:** Click the "Fork" button at the top right of the [GitHub repository](https://github.com/Epic-Nova/Gorgeous-Core). This creates a copy of the project under your GitHub account.
2.  **Clone Your Fork:** Clone the forked repository to your local machine using Git:
    ```bash
    git clone https://github.com/YOUR_USERNAME/Gorgeous-Core.git
    cd Gorgeous-Core
    ```
    Replace `YOUR_USERNAME` with your GitHub username.
3.  **Set Up Your Development Environment:** Follow the instructions in the project's main [`README.md`](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/README.md) or a dedicated [`DEVELOPMENT.md`](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/.github/DEVELOPMENT.md) file (if it exists) to set up your development environment. This might include installing dependencies, configuring your IDE, and setting up any necessary SDKs.
4.  **Create a New Branch:** Before making any changes, create a new branch for your contribution:
    ```bash
    git checkout -b feature/your-feature-name
    ```
    or for bug fixes:
    ```bash
    git checkout -b fix/your-bug-fix-name
    ```
    Choose a descriptive name for your branch.
5.  **Make Your Changes:** Implement your feature, bug fix, or improvement. Follow the [Styleguides](#-styleguides) outlined below.
6.  **Test Your Changes:** Thoroughly test your changes to ensure they work as expected and don't introduce any regressions.
7.  **Commit Your Changes:** Commit your changes with a clear and concise commit message following the [Commit Messages](#-commit-messages) styleguide:
    ```bash
    git add .
    git commit -m "Feat: Add new awesome feature"
    ```
    or
    ```bash
    git commit -m "Fix: Resolve issue with widget rendering"
    ```
8.  **Push to Your Fork:** Push your local branch to your remote fork on GitHub:
    ```bash
    git push origin feature/your-feature-name
    ```
9.  **Create a Pull Request:** Go to the main [GitHub repository](https://github.com/Epic-Nova/Gorgeous-Core) and click the "Compare & pull request" button. Provide a clear and detailed description of your changes in the pull request. Reference any related issues.
10. **Code Review:** Your pull request will be reviewed by the project maintainers. They may provide feedback or ask for changes. Be prepared to address any comments and update your pull request accordingly.
11. **Merging:** Once your pull request is approved, it will be merged into the main branch. Congratulations on your first contribution!

> **Labels on Pull Requests:**
>
> Maintainers will use labels on pull requests to indicate their status and priority. Some labels you might encounter include:
>
> $${\color{darkblue}Noted}$$: Considered important by a contributor, so this request is given a higher priority
> 
> $${\color{steelblue}Triage}$$: Initial review required to determine priority and next steps.
>
> $${\color{forestgreen}Accepted}$$: Accepted by a contributor and is currently in the process of being merged
>
> $${\color{seagreen}Under Review}$$: Under review by a contributor at the moment
>
> $${\color{mediumseagreen}Needs Merge}$$: Preparations to be merged have been completed and it´s now ready to be merged
>
> $${\color{firebrick}Bug Fix}$$: Fixes a bug
>
> $${\color{crimson}Bug Report}$$: Reports a Bug
>
> $${\color{darkorange}Needs Answer from Author}$$: Requires consultation with the author
>
> $${\color{goldenrod}Help Wanted}$$: Attention is needed
>
> $${\color{gold}Needs Update}$$: Needs changes because one or more requirements are not met
>
> $${\color{olive}Optimization}$$: Improves the project's quality or stability
>
> $${\color{teal}New Feature}$$: Not on the roadmap but it´s a good addition and will maybe added to the roadmap
>
> $${\color{darkcyan}Feature}$$: Already on the roadmap and it´s likely to be implemented with some code modifications
>
> $${\color{cadetblue}Feature Request}$$: Community-suggested feature, pending review and roadmap consideration.
>
> $${\color{blue}Documentation}$$: Improvements or additions to the documentation
>
> $${\color{navy}Readme}$$: Improvements or additions in the readme file
>
> $${\color{mediumpurple}Good first Issue}$$: Good for newcomers
>
> $${\color{lavender}Declined}$$: Reviewed and rejected as either not meeting the requirements or contributes nothing to the project
>
> $${\color{thistle}Duplicate}$$: Already exists in itself or in another form
>
> $${\color{lightblue}Invalid}$$: Does not appear to be correct

---

### 📚 Improving The Documentation
We highly value contributions to our documentation! Clear and comprehensive documentation is crucial for the success of Gorgeous Core. Here's how you can help:

-   **Fix Typos and Grammatical Errors:** If you find any mistakes in the documentation, please submit a pull request with the corrections.
-   **Clarify Existing Content:** If you find any sections of the documentation unclear or confusing, please suggest improvements or rewrite them for better understanding.
-   **Add Missing Information:** If you notice any missing information or topics that should be covered, please contribute the missing content.
-   **Improve Examples:** Providing clear and concise examples can greatly enhance the usability of the documentation.
-   **Translate Documentation:** If you are fluent in other languages, you can help by translating the documentation.

Please ensure that your documentation contributions follow the [Documentation Pages](#-documentation-pages) styleguide. Submit your documentation changes as a pull request, just like code contributions.


---

## 🎨 Styleguides

### 🏷️ Naming Conventions

> [!NOTE]
> Consistent naming conventions help maintain code readability and understandability. Please adhere to the following guidelines:

-   **Classes & Files:**
    -   **Blueprint Coding Classes:** Always use the `BP_` prefix. The suffix should always be the capitalized abbreviation of the Unreal Engine class type. For example:
        -   `AActor`: `BP_MyActor_A`
        -   `UActorComponent`: `BP_MyComponent_AC`
        -   `UUserWidget`: `BP_MyWidget_UW`
    -   **C++ Classes:** The `BP_` prefix is not required. Follow standard C++ naming conventions. The capitalized suffix based on the Unreal Engine class type is still recommended for consistency (e.g., `MyActor_A`, `MyComponent_AC`).
    -   **Non-Coding Assets (e.g., Blueprints without code, Materials):** Use a descriptive prefix indicating the asset type (e.g., `M_` for Material, `SM_` for Static Mesh, `T_` for Texture).

-   **General Unreal Naming Convention:** We largely follow the standard Unreal Engine naming conventions outlined here: [Recommended Asset Naming Conventions](https://dev.epicgames.com/documentation/en-us/unreal-engine/recommended-asset-naming-conventions-in-unreal-engine-projects). Please refer to this documentation for asset types not explicitly mentioned above.

-   **Variable Names:** Use `PascalCase` (e.g., `PlayerHealth`, `CurrentLevel`, `TargetLocation`).

-   **Functions/Methods:** Use `PascalCase` (e.g., `InitializePlayer`, `ApplyDamage`, `GetWorldPosition`).

-   **Macro Definitions:** Use `UPPERCASE` with underscores to separate words (e.g., `MAX_HEALTH`, `DEFAULT_SPEED`, `IS_DEBUG_ENABLED`).

> [!TIP]
> **To gain a practical understanding of the Gorgeous Naming Convention, carefully examine the naming of existing files and code elements within the Gorgeous Ecosystem.** Observing these patterns will provide valuable insight into our expectations for naming new contributions.

---

### 💬 Code Comments

> [!NOTE]   
> Well-written code comments are essential for explaining the logic and intent behind your code.

-   **Use comments to explain non-obvious or complex logic.**
-   **Keep comments concise and up-to-date with the code.**
-   **Use block comments (`/* ... */`) for multi-line explanations at the beginning of functions or sections of code.**
-   **Use line comments (`//`) for short explanations of individual lines or small blocks of code.**
-   **Document public APIs (functions, classes, etc.) using docstrings or similar language-specific conventions.** Explain the purpose, parameters, return values, and any potential exceptions.
-   **Avoid stating the obvious.** Comments like `i = i + 1; // Increment i` are not helpful.
-   **Use meaningful variable and function names to reduce the need for excessive commenting.**

> [!TIP]
> **Looking at existing, well-documented code can be a great way to get started.** Pay attention to how experienced developers structure their code and explain their logic through comments. This can provide valuable insights and help you understand best practices for contributing to the project.


---

### 📝 Commit Messages

> [!NOTE]  
> Clear and informative commit messages help track changes and understand the history of the project. Please follow these guidelines:

-   **Separate the subject line from the body with a blank line.**
-   **Limit the subject line to 50 characters.**
-   **Capitalize the subject line.**
-   **Do not end the subject line with a period.**
-   **Use the imperative mood in the subject line** (e.g., "Fix bug", "Add new feature").
-   **Wrap the body at 72 characters.**
-   **Use the body to provide more context and explain the *what* and *why* of the change, not just the *how*.**
-   **Reference any related issues or pull requests in the body** (e.g., "Fixes #123", "Related to #456").

> **Example Commit Messages:**
>
> ```
> Feat: Implement user authentication
>
> This commit introduces user authentication functionality using JWT.
> It includes user registration, login, and logout endpoints.
>
> Fixes #10
> ```
>
> ```
> Fix: Prevent crash when loading corrupted data
>
> This commit addresses an issue where the application would crash
> when attempting to load corrupted user data. It now includes
> error handling to gracefully handle such cases.
> ```

---

### 📚 Documentation Pages

To ensure consistency and clarity across all Gorgeous Core documentation, please adhere to the following guidelines when creating or modifying documentation pages:

* **Clarity and Conciseness:** Use clear, concise, and easy-to-understand language. Avoid jargon or overly technical terms unless absolutely necessary, and provide definitions when needed.

* **Logical Organization:** Structure your documentation logically. Use headings and subheadings to break down information into manageable sections. Consider using a table of contents for longer documents.

* **Code Examples:** Where applicable, provide clear and well-formatted code examples. Ensure that examples are accurate, complete, and easy to copy and paste. Explain the purpose and expected output of each example.

* **Consistent Terminology:** Use consistent terminology throughout the documentation. If a concept has a specific name within Gorgeous Core, use that name consistently.

* **Consistent Formatting:** Maintain a consistent formatting style for headings, code blocks, lists, and other elements. Use Markdown consistently.

* **Links and References:** Link to other relevant sections of the documentation, external resources, or the Unreal Engine documentation where appropriate. Provide context for each link.

* **Visual Aids:** Consider using diagrams, screenshots, or screen recordings to illustrate complex concepts or procedures. Ensure that visual aids are clear and relevant.

* **Proofreading:** Always proofread your documentation carefully for typos, grammatical errors, and clarity. Ask a colleague to review your documentation if possible.

* **Target Audience:** Consider your target audience when writing documentation. Are you writing for beginners, experienced users, or developers? Tailor your language and level of detail accordingly.

* **Up-to-Date Information:** Ensure that your documentation is kept up-to-date with the latest version of Gorgeous Core. Review and update documentation whenever significant changes are made to the project.

* **Example Structure:** Consider structuring your documentation pages as follows:

    1.  **Introduction:** Briefly introduce the topic and its purpose.
    2.  **Overview:** Provide a high-level overview of the concept or feature.
    3.  **Usage:** Explain how to use the concept or feature, providing code examples where appropriate.
    4.  **Parameters/Inputs/Outputs:** Clearly define any parameters, inputs, or outputs.
    5.  **Best Practices:** Offer guidance on best practices for using the feature or concept.
    6.  **Troubleshooting:** Include a section on common issues and how to resolve them.
    7.  **Related Information:** Link to other relevant documentation or resources.

By following these guidelines, we can ensure that the Gorgeous Core documentation is clear, consistent, and helpful for all users.

---

## 👥 Join The Project Team
We're always excited to welcome passionate and dedicated individuals to the Gorgeous Core project team! If you're looking to get more deeply involved, contribute on a regular basis, and help shape the future of this project, we encourage you to consider joining us.

**Here are some ways you can express your interest in joining the team:**

* **Consistent Contributions:** Demonstrate your commitment by regularly contributing in areas that interest you, whether it's through code contributions, bug fixes, documentation improvements, or helping to answer community questions.
* **Active Participation:** Engage actively in discussions on GitHub issues, pull requests, and any community forums we might have. Share your insights and ideas.
* **Proactive Identification of Needs:** Show initiative by identifying areas where you can contribute value, whether it's tackling unassigned issues, proposing new features, or improving existing processes.
* **Communication and Collaboration:** Be a good team player by communicating clearly, respectfully, and collaborating effectively with other contributors and maintainers.

**The process for joining the team typically involves the following steps:**

1.  **Express Your Interest:** Reach out to the project maintainers (you can mention your interest in a relevant issue, a pull request, or by sending an email to [support@epicnova.net](mailto:support@epicnova.net) with the subject "Interest in Joining the Team"). Briefly explain your background, areas of interest, and how you'd like to contribute more significantly.
2.  **Demonstrate Your Capabilities:** Continue to contribute consistently and demonstrate your skills and understanding of the project's goals and standards.
3.  **Discussion with the Team:** The existing project team will discuss your interest and contributions. We'll consider your technical skills, communication abilities, and commitment to the project.
4.  **Invitation:** If there's a good fit, you'll receive an invitation to join the team with appropriate responsibilities and access rights.

**Benefits of joining the team:**

* **Direct Impact:** You'll have a more direct impact on the project's direction and development.
* **Deeper Collaboration:** You'll work more closely with core contributors and gain valuable experience.
* **Increased Responsibility:** You may be given more responsibilities, such as reviewing contributions or managing specific areas of the project.
* **Recognition:** Your contributions as a team member will be more prominently recognized.

We value diverse perspectives and are looking for individuals who are passionate about Gorgeous Core and eager to help it grow. If you think you have what it takes, we'd love to hear from you!

---

## Attribution
This guide is based on the [contributing.md](https://contributing.md/generator)!