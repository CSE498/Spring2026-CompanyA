# Installing CMake

This guide covers how to install CMake on Linux, WSL, Windows, and macOS, and how to check whether it is already installed first.

---

## Check if CMake is already installed

Before installing, run:

```bash
cmake --version
```

If CMake is installed, you should see output similar to:

```bash
cmake version 3.x.x
```

If you get “command not found” or a similar error, continue with the install steps below.

---

# Linux

## Check if installed

```bash
cmake --version
```

## Ubuntu / Debian

```bash
sudo apt update
sudo apt install cmake
```

## Fedora

```bash
sudo dnf install cmake
```

## Arch Linux

```bash
sudo pacman -S cmake
```

## openSUSE

```bash
sudo zypper install cmake
```

## Verify installation

```bash
cmake --version
```

---

# WSL

Install CMake **inside your WSL Linux distribution**, not through normal Windows Command Prompt or PowerShell for WSL project builds.

## Check if installed

Inside the WSL terminal, run:

```bash
cmake --version
```

## Ubuntu / Debian on WSL

```bash
sudo apt update
sudo apt install cmake
```

## Verify installation

```bash
cmake --version
```

## Confirm you are in WSL

```bash
uname -a
```

You should see output mentioning `Linux`, and often `microsoft` or `WSL`.

---

# Windows

## Check if installed

In **PowerShell** or **Command Prompt**, run:

```powershell
cmake --version
```

You can also check where it is being found from:

```powershell
where cmake
```

## Install with winget

```powershell
winget install Kitware.CMake
```

## Install with Chocolatey

```powershell
choco install cmake
```

## Manual installer

1. Go to the official CMake download page from Kitware.
2. Download the Windows installer.
3. During setup, choose the option to add CMake to your `PATH`.

## Verify installation

Close and reopen your terminal, then run:

```powershell
cmake --version
```

---

# macOS

## Check if installed

```bash
cmake --version
```

You can also check where it is installed from:

```bash
which cmake
```

## Install with Homebrew

```bash
brew install cmake
```

## Verify installation

```bash
cmake --version
```

---

# Extra troubleshooting

## If `cmake` is installed but not found

It may not be on your `PATH`.

### Linux / WSL / macOS

Try:

```bash
which cmake
```

### Windows

Try:

```powershell
where cmake
```

If nothing is returned, reinstall CMake and make sure it is added to your `PATH`.

## If you need a newer version

Some Linux package managers may provide an older CMake version. In that case, install from the official Kitware release instead of the default distro package.

---

# Recommended quick check section for issue reports

If someone is reporting a build issue, ask them to include:

```bash
cmake --version
```

and, if relevant:

```bash
which cmake
```

or on Windows:

```powershell
where cmake
```
