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

### Conda?

It looks like this can be installed via conda (Linux only). I haven't tested this... Someone should at the workshop:
```bash
mamba install -c conda-forge apptainer
```

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




# What to do when Apptainer **cannot** run a Docker image?

There are some fringe cases when Apptainer cannot successfully run a Docker image as intended. 
When this happens there are a few steps we can take.

## Unsure what's going on? -> Brain Surgery! (Create a sandbox)

We can create a sandbox, or a writable image using:
```bash
apptainer build --sandbox sandbox_directory image_name
```
This will create a sandbox version of the `image_name` image in the `sandbox_directory`. For example:
```bash
apptainer build --sandbox python_project docker://python
```
This will create a directory called `python_project`. 
Within this directory we see what is the typical contents of `/`:
```bash
> ls python_project 
bin  boot  dev  environment  etc  home  lib  lib64  media  mnt  opt  proc  root  run  sbin  singularity  srv  sys  tmp  usr  var
```
This is essentially the filesystem snapshot that we load in when we create a container.

We can then start the container as writable using:
```bash
apptainer shell --writable python_project
```
This starts a new container based on the `python_project` directory. We can make changes to this container, for example installing `ipython`:
```bash
pip install ipython
```
Once we exit the container, the changes are saved to the `python_project` directory. 
We can then create a compressed image using:
```bash
apptainer build python_project.sif python_project 
```

## Environmental Variables and Setup

One of the differences between Docker and Apptainer, is that when you run a Docker container you essentially start from the last step defined in the Dockerfile. This means that steps that going into the Dockerfile, for example sourcing an environmental file, remains active in the container.

This isn't the case in Apptainer. Apptainer essentailly starts a new shell/instance. Apptainer uses a `%environment` section to define environmental variables and a `%runscript` to define what happens when we enter a container to run a command, or `%startscript` to define what happens when starting an instance.

Some common examples of this are:
* Sourcing environmental variables
* Activating python environments

Let's look at examples of how to solve these.

### Sourcing environmental variables

A lot of software that is built ontop of ROOT and Geant4 require various scripts to be sourced prior to running the code. 
For example the Helix-env image.
Because the various sourcing is done within the Dockerfile, this isn't passed to Apptainer. Let's start off by looking in the container:
```bash
apptainer run helix.sif cat /cern/helix_env.sh
```
Which shows the following:
```
# A stub you can source to get your cern stuff defined
# You may need to do it in your env file
source /cern/root/bin/thisroot.sh
if [ -e /cern/g4 ]; then
  export G4INSTALL=/cern/g4
  export LWD=$PWD
  cd /cern/g4/bin
  source geant4.sh
  cd $LWD
fi
```
So we need to run `source /cern/helix_env.sh` in order to "source" the environment. Perhaps we can do this using the `%environment`?
```bash
Bootstrap: localimage
From: ./helix.sif
Stage: build

%environment
    source /cern/helix_env.sh
```
Adding to the `%environment` produces the following error:
```
source: /cern/root/bin/thisroot.sh:209:13: parameter expansion requires a literal
```
We can potentially solve this by appending the `.bashrc` file with the contents... but remember, the container user is us and we're using the same `.bashrc` file. We don't want to modify the host system!

So this didn't work, let's try to use a `%runscript` instead:
```bash
Bootstrap: localimage
From: ./helix.sif
Stage: build

%runscript
    #!/bin/bash
    source /cern/helix_env.sh
    exec $*
```

Here the runscript first runs `source /cern/helix_env.sh` before running `exec $*`. The second command will simply execute any arguments that where passed to the script.

We can test this now with:
```bash
apptainer run helix2.sif root-config --version
```

Note here: `root-config --version` is passed as arguments to the `%runscript`. These arguments are captured in `$*` and passed to `exec`. The `%runscript` is effectively now:
```bash
#!/bin/bash
source /cern/helix_env.sh
root-config --version
```

### Activating Python environment

Let's define the `%environment` of a container using conda to run a sub environment:
```
Bootstrap: docker
From: condaforge/mambaforge

%environment
    # set up environment for when using the container
    . /opt/conda/etc/profile.d/conda.sh
    conda activate
    conda activate test_env

%post
    mamba create --name test_env python=3.8
    # mamba init
    # conda activate test_env


# %runscript
#     #!/bin/bash
#     conda activate test_env
```

In general, you shouldn't need to create a new python environment. Instead one could install all the packages they need within the base environment within the image.



## Cache and User install locations

