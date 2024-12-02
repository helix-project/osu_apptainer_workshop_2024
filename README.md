# Apptainer

[Apptainer](https://apptainer.org/) (formerly called [Singularity](https://docs.sylabs.io/guides/3.5/user-guide/introduction.html)) is a containerization software often used in High Performance Computing (HPC) environments.
This is partly due to security issues when running Docker on a multiuser system. 
Apptainer addresses these security concerns by insisting that the user within the container is the same user that created and is running a process within the container.
This means that if User A runs a container, within the container the user has the same permissions as User A. This means that User B's files cannot be modified by someone else running a container.
This would be possible using Docker as User A could create a user within the container with the permissions of User B.
This also means that the owness is on the user to make sure that their files are appropriately managed.

## Installing Apptainer

### Linux

To install on Linux, follow instructions [here](https://apptainer.org/docs/admin/main/installation.html#installation-on-linux).

### Windows/Mac installation

Installing on Windows/Mac can be a little convoluted. Simply put, Windows/Mac installation requires that Apptianer be run through a Linux virtual machine/container. 

For Windows, the easiest way to do this is through WSL. Instructions can be found [here](https://apptainer.org/docs/admin/main/installation.html#windows).

For Mac, the recommended way is to install via `lima` which can be installed using Homebrew. Instructions can be found [here](https://apptainer.org/docs/admin/main/installation.html#mac).

## Apptainer Shell

Let's jump right in and try to run an image using Apptainer. We're lazy, so we're going to rely on other's work.
In particular, we'll grab an image from [Docker Hub](https://hub.docker.com/). Let's look at [rootproject's ROOT image](https://hub.docker.com/r/rootproject/root). 
When we can start a new containerized shell from an image using the `shell` keyword:
```bash
apptainer shell docker://rootproject/root
```

Where we specify that we want a "shell". We then specify the image type. In particular, we're specifying that we have a docker image (`docker://`) and that we're using the rootproject's root image (`rootproject/root`).
We could have further specified the version with something like:
```bash
apptainer shell docker://rootproject/root:6.26.02-ubuntu22.04 
```
Which will grab the `6.26.02-ubuntu22.04` version. 

We'll notice that if the image isn't found on our host system, apptainer will download the image from the provided URL. Let's say we want to keep a local copy of the image that won't get deleted if we decide to clear our caches we can use `apptainer pull` to store a local copy.

## Apptainer Pull

By running:
```bash
apptainer pull imagename
```
We'll download `imagename` and store it into apptainer's cache (usually `~/.apptainer` by default). Let's say we want to store a more permanent copy of the image. We could download and save to a `sif` file using:
```bash
apptainer pull imagename.sif imagename
```
Notice how the `imagename.sif` is the first argument after `pull`. In the case of rootproject's ROOT image:
```bash
apptainer pull root.sif docker://rootproject/root
```
This will create a file called `root.sif`:
```bash
ls -lah ./*.sif                                                                                                    [16:54:23]
-rwxrwxr-x 1 obriens obriens 729M Dec  2 16:54 ./root.sif
```
Note the file size compared to the docker image:
```bash
docker image ls | grep root                                                                                        [16:55:31]
rootproject/root                     latest     34eb35079484   2 months ago    2.76GB
```

This is one of the first major advantages of using apptainer, the file size. 

## Apptainer Run

Like docker, we can use a default run command when working with apptainer. As we saw in the docker tutorial, the default run command of the root image is `root -b`. This launches a root interpreter. We can run this using:
```bash
apptainer run root.sif
```

Alternatively we can also run the `.sif` file as an executable:
```bash
./root.sif
```

If we want to overwrite the default command, we can pass another command at the end. For example, let's print the working (current) directory using `pwd`:
```bash
apptainer run root.sif pwd
```

You might notice that apptainer sees the current directory. This is unlike Docker, which is completely containerized and separate file system. By default, apptainer mounts a number of commonly used directories such as `/home`, `/tmp` and `/dev`, see [here](https://apptainer.org/docs/user/main/bind_paths_and_mounts.html) for a full list. We can override this by passing `--no-mount`:

```bash
apptainer run --no-mount root.sif pwd
```
This will likely give a file permission error as apptainer tries to access a folder that it no longer has access too.




# Where Docker doesn't translate into Apptainer

## Unsure what's going on? -> Brain Surgery!

## Environmental Variables and Setup

## Cache and User install locations

