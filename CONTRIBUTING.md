# Contributing to Gorgeous Core

First off, thanks for taking the time to contribute! ❤️

All types of contributions are encouraged and valued. See the [Table of Contents](#table-of-contents) for different ways to help and details about how this project handles them. Please make sure to read the relevant section before making your contribution. It will make it a lot easier for us maintainers and smooth out the experience for all involved. The community looks forward to your contributions. 🎉

> And if you like the project, but just don't have time to contribute, that's fine. There are other easy ways to support the project and show your appreciation, which we would also be very happy about:
> - Star the project
> - Refer this project in your project's readme
> - Mention the project at local meetups and tell your friends/colleagues
> - Download the plugin from the Fab Marketplace

## Table of Contents

- [I Have a Question](#i-have-a-question)
- [I Want To Contribute](#i-want-to-contribute)
    - [Reporting Bugs](#reporting-bugs)
    - [Reporting Vulnerabilities](#reporting-vulnerabilities)
    - [Suggesting Enhancements](#suggesting-enhancements)
    - [Your First Code Contribution](#your-first-code-contribution)
    - [Improving The Documentation](#improving-the-documentation)
- [Styleguides](#styleguides)
    - [Naming Conventions](#naming-conventions)
    - [Code Comments](#code-comments)
    - [Commit Messages](#commit-messages)
    - [Documentation Pages](#documentation-pages)
- [Join The Project Team](#join-the-project-team)

---

## I Have a Question

> If you want to ask a question, we assume that you have read the available [Documentation](https://epic-nova.github.io/Gorgeous-Core/).

Before you ask a question, it is best to search for existing [Issues](https://github.com/Epic-Nova/Gorgeous-Core/issues) that might help you. In case you have found a suitable issue and still need clarification, you can write your question in this issue. It is also advisable to search the internet for answers first.

If you then still feel the need to ask a question and need clarification, we recommend the following:

- Read the [FAQ.md](https://github.com/Epic-Nova/Gorgeous-Core/blob/master/FAQ.md)
- Open an [Question Issue](https://github.com/Epic-Nova/Gorgeous-Core/issues/new?template=question.md).
- Write an email to [support@epicnova.net](mailto:support@epicnova.net)

We will then take care of the issue as soon as possible.

<!--
You might want to create a separate issue tag for questions and include it in this description. People should then tag their issues accordingly.

Depending on how large the project is, you may want to outsource the questioning, e.g. to Stack Overflow or Gitter. You may add additional contact and information possibilities:
- IRC
- Slack
- Gitter
- Stack Overflow tag
- Blog
- FAQ
- Roadmap => Project board (reference this)
- E-Mail List
- Forum
-->

---

## I Want To Contribute

> ### Legal Notice
> When contributing to this project, you must agree that you have authored 100% of the content, that you have the necessary rights to the content and that the content you contribute may be provided under the project licence.

### Reporting Bugs

#### Before Submitting a Bug Report

A good bug report shouldn't leave others needing to chase you up for more information. Therefore, we ask you to investigate carefully, collect information and describe the issue in detail in your report. Please complete the following steps in advance to help us fix any potential bug as fast as possible.

- Make sure that you are using the latest version.
- Determine if your bug is really a bug and not an error on your side e.g. using incompatible environment components/versions (Make sure that you have read the [documentation](https://epic-nova.github.io/Gorgeous-Core/). If you are looking for support, you might want to check [this section](#i-have-a-question)).
- To see if other users have experienced (and potentially already solved) the same issue you are having, check if there is not already a bug report existing for your bug or error in the [bug tracker](https://github.com/Epic-Nova/Gorgeous-Core/issues).
- Also make sure to search the internet (including Stack Overflow) to see if users outside of the GitHub community have discussed the issue.
- Collect information about the bug:
- Stack trace (Traceback)
- OS, Platform and Version (Windows, Linux, macOS, x86, ARM)
- Version of the engine, compiler, SDK, runtime environment.
- Possibly your input and the output
- Can you reliably reproduce the issue? And can you also reproduce it with older versions?

#### How Do I Submit a Good Bug Report?

> You must never report security related issues, vulnerabilities or bugs including sensitive information to the issue tracker, or elsewhere in public. How to handle vulnerability reporting is described [here](#reporting-vulnerabilities).

We use GitHub issues to track bugs and errors. If you run into an issue with the project:

- Open an [Issue](https://github.com/Epic-Nova/Gorgeous-Core/issues/new?template=bug_report.md).
- Explain the behavior you would expect and the actual behavior.
- Please provide as much context as possible and describe the *reproduction steps* that someone else can follow to recreate the issue on their own. This usually includes your code. If the code that you are experiencing issues with, is Blueprint, then copy your script code and paste it in [BlueprintUE](https://blueprintue.com/) and share the link in the issue. For good bug reports you should isolate the problem and create a reduced test case. 
- Provide the information you collected in the previous section.

Once it's filed:

- The project team will label the issue accordingly.
- A team member will try to reproduce the issue with your provided steps. If there are no reproduction steps or no obvious way to reproduce the issue, the team will ask you for those steps and mark the issue as <span style="color:#FBCA04">#08 Needs Update</span>. Bugs with the <span style="color:#FBCA04">#08 Needs Update</span> tag will not be addressed until they are reproduced.
- If the team is able to reproduce the issue, then the <span style="color:#0774b3">#01.1 Triage</span> tag will be removed to indicate that this is a valid bug report. Also it will be marked possibly with other tags (such as <span style="color:#032B63">#01 Noted</span>), and the issue will be left to be [implemented by someone](#your-first-code-contribution).

---

### Reporting Vulnerabilities
<!-- TODO
Should be reported to: vulnerabilities@epicnova.net
PGP Key to sent these infomation encrypted: https://github.com/Epic-Nova/Gorgeous-Core/blob/master/SensitiveReportingPublicKey.asc
-->

---

### Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for Gorgeous Core, **including completely new features and minor improvements to existing functionality**. Following these guidelines will help maintainers and the community to understand your suggestion and find related suggestions.

#### Before Submitting an Enhancement

- Make sure that you are using the latest version.
- Read the [documentation](https://epic-nova.github.io/Gorgeous-Core/) carefully and find out if the functionality is already covered, maybe by an individual configuration.
- Perform a [search](https://github.com/Epic-Nova/Gorgeous-Core/issues) to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.
- Find out whether your idea fits with the scope and aims of the project. It's up to you to make a strong case to convince the project's developers of the merits of this feature. Keep in mind that we want features that will be useful to the majority of our users and not just a small subset.

#### How Do I Submit a Good Enhancement Suggestion?

Enhancement suggestions are tracked as [Feature issues](https://github.com/Epic-Nova/Gorgeous-Core/issues/new?template=feature_request.md).

- Use a **clear and descriptive title** for the issue to identify the suggestion.
- Provide a **step-by-step description of the suggested enhancement** in as many details as possible.
- **Describe the current behavior** and **explain which behavior you expected to see instead** and why. At this point you can also tell which alternatives do not work for you.
- You may want to **include screenshots or screen recordings** which help you demonstrate the steps or point out the part which the suggestion is related to. You can use [LICEcap](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and the built-in [screen recorder in GNOME](https://help.gnome.org/users/gnome-help/stable/screen-shot-record.html.en) or [SimpleScreenRecorder](https://github.com/MaartenBaert/ssr) on Linux. <!-- this should only be included if the project has a GUI -->
- **Explain why this enhancement would be useful** to most Gorgeous Core users. You may also want to point out the other projects that solved it better and which could serve as inspiration.

---

### Your First Code Contribution
<!-- TODO
include Setup of env, IDE and typical getting started instructions?

Mention here the Labels that we have and when what labels will be added to a pull request

use the styleguide
-->

---

### Improving The Documentation
<!-- TODO
Updating, improving and correcting the documentation

use the styleguide

-->

---

## Styleguides

### Naming Conventions
<!-- TODO

-->

---

### Code Comments
<!-- TODO

-->

---

### Commit Messages
<!-- TODO

-->

---

### Documentation Pages
<!-- TODO

-->

---

## Join The Project Team
<!-- TODO -->

---

## Attribution
This guide is based on the [contributing.md](https://contributing.md/generator)!