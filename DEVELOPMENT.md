# 👨‍💻 DEVELOPMENT.md

Welcome to the **Gorgeous Core Development Guide!** This document will walk you through setting up your development environment to work with the Gorgeous Core plugin in **Unreal Engine** — whether you're a Blueprint-only user or a full-on C++ developer.

> 🧠 **Before You Begin:**  
> Make sure to read the [README.md](./README.md) for an overview of the project and contribution guidelines.

--- 
## 🚀 Getting Started with Unreal Engine

### 📦 Install Unreal Engine

Gorgeous Core is developed and tested with **Unreal Engine 5.3+**.

To install Unreal Engine:

1. Download and install the **Epic Games Launcher**:  
   👉 [https://www.unrealengine.com/download](https://www.unrealengine.com/download)

2. Open the launcher and install your desired version of **UE5** from the **Library** tab.

> [!NOTE]
> 💡 **We recommend using the latest stable release.** Gorgeous Core aims to stay up-to-date with the newest versions.

---

## 🟦 Blueprint-Only Users (No C++ Required)

If you're not planning to compile code and only want to **use the plugin in Blueprints**, follow these steps:

### ✅ Step 1: Download Precompiled Plugin

Download the latest **compiled plugin binaries** from the [Releases](https://github.com/Epic-Nova/Gorgeous-Core/releases) section of this repository.

- Choose the `.zip` file that matches your **Unreal Engine version**.
- Unzip the contents into your project’s `Plugins/` folder (create one if it doesn’t exist).

```md
YourProject/
└── Plugins/
    └── GorgeousCore/
```

### ✅ Step 2: Enable the Plugin

1. Open your project in **Unreal Engine**.
2. Go to `Edit > Plugins`.
3. Search for **Gorgeous Core**.
4. Click **Enable**.
5. Restart the editor.

### ✅ Step 3: Configure the Plugin

You're all set! You can now start using the features of Gorgeous Core in your Blueprints.  
Explore the **Examples**, check the **Docs**, and get building! 🎮

---

## 💻 C++ Developers

Want to dive deeper or contribute to the codebase? Let’s get your environment fully set up.

### 🔽 Step 1: Download the Unreal Engine Source (Optional)

You *can* use the launcher version, but if you want full engine control or custom modifications:

- Follow Epic’s official guide:  
  👉 [Downloading Unreal Engine from Source](https://docs.unrealengine.com/en-US/downloading-unreal-engine-source-code)

  👉 [Building Unreal Engine from Source](https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source)


> [!NOTE]
> ⚠️ Building UE from source takes time and disk space. Use it if you're modifying engine internals.

---

### 🧰 Step 2: Set Up Your IDE

#### 🪟 For Windows Users

Install **Visual Studio 2022 or newer** with these workloads:

- ✔️ **Game development with C++**
- ✔️ **Desktop development with C++**

Make sure these components are also installed:

- ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools
- ✅ Windows 10 SDK or Windows 11 SDK
- ✅ CMake Tools for Windows

> [!TIP]
> 💡 Don't worry if you missed something — the engine will prompt you when you try to compile.

#### 🍎 For macOS Users

For macOS, you’ll need to install **Xcode** to work with Unreal Engine. Follow these steps:

1. Install **Xcode** from the [Mac App Store](https://apps.apple.com/us/app/xcode/id497799835?mt=12).
2. Open **Xcode** and agree to the license terms when prompted.
3. Install the **Xcode Command Line Tools** via Terminal:
    ```bash
    xcode-select --install
    ```

> [!NOTE]
> ⚠️ Xcode is required for compiling and building Unreal Engine projects on macOS.

After installing Xcode, make sure to also have the following:

- **CMake** (Usually installed with Xcode or via [Homebrew](https://brew.sh/)):  
  To install CMake, use the following command:
    ```bash
    brew install cmake
    ```
---

### 💡 Optional: Use Rider for Unreal

JetBrains Rider is a fantastic free IDE for Unreal Engine development:  
👉 [Download Rider for Unreal Engine](https://www.jetbrains.com/rider/unreal/)

Rider offers:

- 🚀 **Fast indexing**
- 🧠 **Smart navigation**
- 🤖 **C++/Blueprint hybrid tooling**

---

### 📁 Step 3: Download Plugin Source

Clone or download this repository, and copy the **`GorgeousCore/`** folder into your project’s **`Plugins/`** directory:

```bash
YourProject/
└── Plugins/
    └── GorgeousCore/  ← put source files here
```

---

### ⚙️ Step 4: Generate Project Files

#### 🪟 For Windows Users

Right-click your .uproject file and choose:

```
Generate Visual Studio project files
```
> [!TIP]
> If you're using **Rider**, open the .uproject and let it handle the setup.

#### 🍎 For macOS Users

On macOS, you'll need to generate project files **using the Unreal Engine terminal commands**. Follow these steps:

1. Open a **Terminal** window.
2. Navigate to your Unreal Engine project folder:
    ```bash
    cd /path/to/your/project
    ```
3. Run the following command to generate project files:
    ```bash
    sh "/Users/Shared/Epic\ Games/UE_5.XX/Engine/Build/BatchFiles/Mac/GenerateProjectFiles.sh"
        -project="/path/to/your/project/YourProject.uproject" -game
    ```

> [!NOTE]
> 💡 Replace `/path/to/your/project` with the actual paths on your system.

This will generate the necessary project files for **Xcode** to open and compile your project.

> [!TIP]
> If you're using **Rider**, simply open the `.uproject` file, and Rider will automatically set everything up for you.

---
 
### 🛠️ Step 5: Build the Project

Open your project in Visual Studio or Rider and compile it:

- Use the **Development Editor** configuration
- Target your preferred platform (typically **Win64**)

After the build succeeds, launch the editor!

---

### ✅ Step 6: Enable and Configure

Once inside Unreal:

1. Navigate to ```Edit -> Plugins```.
2. Search for **Gorgeous Core**.
3. Enable it, restart the editor, and start exploring this Plugin!

---

### 🧪 Need Help?

Check the [Issues](https://github.com/Epic-Nova/Gorgeous-Core/issues), ask questions, or reach out to the dev team. We’re happy to help you get started and contribute!

### 🙌 You're Ready!

Whether you're just using the plugin or helping us build it — welcome aboard.
Thanks for being part of the **Gorgeous Core** journey! 🌌