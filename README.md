# Apptainer

[Apptainer](https://apptainer.org/) (formerly called [Singularity](https://docs.sylabs.io/guides/3.5/user-guide/introduction.html)) is a containerization software often used in High Performance Computing (HPC) environments.
This is partly due to security issues when running Docker on a multiuser system. 
Apptainer addresses these security concerns by insisting that the user within the container is the same user that created and is running a process within the container.
This means that if User A runs a container, within the container the user has the same permissions as User A. This means that User B's files cannot be modified by someone else running a container.
This would be possible using Docker as User A could create a user within the container with the permissions of User B.
This also means that the owness is on the user to make sure that their files are appropritately managed.

## Installing Apptainer

### Linux

To install on linux, follow instructions [here](https://apptainer.org/docs/admin/main/installation.html#installation-on-linux).

### Windows/Mac installation

Installing on Windows/Mac can be a little convoluted. Simply put, Windows/Mac installation requires that Apptianer be ran through a linux virtual machine/container. 

For Windows, the easiest way to do this is through WSL. Instructions can be found [here](https://apptainer.org/docs/admin/main/installation.html#windows).

For Mac, the recommended way is to install via `lima` which can be installed using homebrew. Instructions can be found [here](https://apptainer.org/docs/admin/main/installation.html#mac).

## Apptainer Shell

Let's jump right in and try to run an image using apptainer. We're lazy, so we're gonna rely on other's work.
In particular we'll grab an image from [Docker Hub](https://hub.docker.com/). Let's look at [rootproject's ROOT image](https://hub.docker.com/r/rootproject/root). 
When we can start a new containerized shell from an image using the `shell` keyword:
```bash
apptainer shell docker://rootproject/root
```

Where we specify that we want a "shell". We then specify the image type. In particular we're specifying that we have a docker image (`docker://`) and that we're using the rootproject's root image (`rootproject/root`).
We could of further specified the version with something like:
```bash
apptainer shell docker://rootproject/root:6.26.02-ubuntu22.04 
```
Which will grab the `6.26.02-ubuntu22.04` version. 