# 9pfsPkg

9pfsPkg is a Plan 9 file system protocol (9P) client for UEFI. In contrast to other network boots, it provides `EFI_SIMPLE_FILE_SYSTEM_PROTOCOL` interfaces for network-transparent file system operations. Any existing non-network-aware UEFI applications including bootloaders and Linux can use for 9P Boot without any modification.

![9P Boot Overview](https://retrage.github.io/img/9pfspkg-9p-boot-overview.png)

By coping with other file systems on the 9P server, 9pfsPkg enables Proxy Boot, network boot from another complicated file system like cloud storage.

![Proxy Boot Overview](https://retrage.github.io/img/9pfspkg-proxy-boot-overview.png)

For more details, see the [blog post](https://retrage.github.io/2020/08/01/9pfspkg-en.html).

## Build 9pfsPkg

9pfsPkg requires [EDK II](https://github.com/tianocore/edk2).

```
# Clone EDK II repository.
$ git clone https://github.com/tianocore/edk2.git
$ cd edk2
$ git submodule update --init --recursive

# Clone 9pfsPkg in the EDK II root directory.
$ git clone https://github.com/yabits/9pfsPkg.git


# Build build tools and set environment variables.
$ make -C BaseTools
$ source ./edksetup.sh

# Build 9pfsPkg.
$ build -p 9pfsPkg/9pfsPkg.dsc
```

## Usage

To use `9pfs.efi`, set the following UEFI variables with GUID `g9pfsGuid` (`10e4a8f-ed0a-4eed-85ee-216a9d3f092e`).
It is recommended to create a helper application to set them.

* `StationAddr`:  Client IPv4 address in CHAR16 (e.g. `L"10.0.2.2:564"`)
* `SubnetMask`:   Client IPv4 subnet mask in CHAR16 (e.g. `L"255.255.255.0"`)
* `RemoteAddr`:   9P server IPv4 address in CHAR16 (e.g. `L"10.0.2.100:564"`)
* `UName`:        Access user name in CHAR8 (e.g. `"root"`)
* `AName`:        Exported directory path in CHAR8 (e.g. `"/tmp/9"`)

```
# Load 9pfsPkg UEFI driver.
FS0:\> load 9pfs.efi

# Update map.
FS0:\> map -u

# Move to the volume.
FS0:\> fs1:
```

## Proxy Boot

Proxy Boot is one of the 9pfsPkg applications. It is booting from another file system via the 9P server. As an example of Proxy Boot, we use Google Cloud Storage (GCS) for the network boot using [gcsfuse](https://github.com/GoogleCloudPlatform/gcsfuse).

```
# Create a GCS bucket and upload boot images.

# Mount the GCS bucket.
$ sudo -E gcsfuse proxy-boot /mnt/gcs
Using mount point: /mnt/gcs
Opening GCS connection...
Opening bucket...
Mounting file system...
File system has been successfully mounted.
```

The below example boots [BitVisor](https://sourceforge.net/projects/bitvisor/) from the GCS bucket.

```
Shell> fs0:
FS0:\> load 9pfs.efi
FS0:\> map -u
FS0:\> fs1:
FS1:\> loadvmm.efi
Starting BitVisor...
Copyright (c) 2007, 2008 University of Tsukuba
All rights reserved.
```

## License

9pfsPkg is released under the [BSD-2-Clause Plus Patent License](LICENSE).