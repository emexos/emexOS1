# Installing QEMU on Arch-based Linux Distributions

This guide explains how to install QEMU and related virtualization tools on Arch Linux and Arch-based distributions, such as Manjaro or EndeavourOS.

## Step 1: Update your system

Before installing new packages, it is recommended to update your package repositories and upgrade existing packages. Open a terminal and run:

```sh
sudo pacman -Syu
```

## Step 2: Install QEMU and Virtualization Utilities

To set up QEMU along with essential virtualization tools, install the following packages:

- `qemu`: The main QEMU emulator package, supports various architectures
- `virt-manager`: A graphical user interface for managing virtual machines
- `bridge-utils`: Tools for configuring network bridging for VMs
- `dnsmasq` and `vde2`: (Optional) Additional networking tools recommended for advanced network setup

Install with the following command:

```sh
sudo pacman -S qemu virt-manager bridge-utils dnsmasq vde2
```

If prompted, confirm installation of dependencies.

## Step 3: Enable and start required services

For networking and virtualization, enable and start the `libvirtd` service:

```sh
sudo systemctl enable --now libvirtd.service
```

Add your user to the `libvirt` and `kvm` groups to manage virtual machines without root privileges:

```sh
sudo usermod -aG libvirt,kvm $USER
```

**Log out and log back in** after adding yourself to these groups.

## Step 4: Verify installation

Check that QEMU is installed correctly by running:

```sh
qemu-system-x86_64 --version
```

Open Virtual Machine Manager (virt-manager) via your applications menu or by entering:

```sh
virt-manager
```

You should now be able to create and run virtual machines using QEMU on your Arch-based system.

---

**For additional documentation and troubleshooting, consult:**

- [Arch Wiki: QEMU](https://wiki.archlinux.org/title/QEMU)
- [Arch Wiki: Libvirt](https://wiki.archlinux.org/title/Libvirt)
- [Arch Wiki: Virt-manager](https://wiki.archlinux.org/title/Virt-manager)
