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

## Binding Volumes

We can also bind volumes to custom locations within the container. This is useful when working with pipelines.

```bash
apptainer run --bind $(pwd):/data_dir root.sif ls /data_dir
```

We can explicitly bind the mount as read only by adding `:ro` to the bind command:
```bash
> apptainer run --bind $(pwd):/data_dir:ro root.sif touch /data_dir/test.txt
/usr/bin/touch: cannot touch '/data_dir/test.txt': Read-only file system
```

## Creating an image from a definition file

Similar to Docker, we can create an image from a file with a list of instructions.

Apptainer `def` files follow this format:
```
Bootstrap: docker
From: ubuntu:{{ VERSION }}
Stage: build

%arguments
    VERSION=22.04

%setup

%files

%environment

    
%post


%runscript

%startscript
    
%test

%labels

%help

```

We'll walk through these sections one by one.

### Preamble

```
Bootstrap: docker
From: ubuntu
Stage: build
```

* `Bootstrap` specifies where we are getting the base image from. In this case, it's `docker` (DockerHub).
* `From:` specifies the base image. In this case, it will grab the latest Ubuntu image.
* `Stage:` specifies the stage of the build. Multiple stages can be used to simplify the build process and reduce the final file size.


### `%arguments`

```
Bootstrap: docker
From: ubuntu:{{ VERSION }}
Stage: build

%arguments
    VERSION=22.04
```

Arguments are variables that can be used within the definition file. Using arguments allows us to change variables only in one place rather than multiple instances, preventing bugs.

In the above example, we've specified an argument `VERSION=22.04`. This argument is then accessed in the preamble when selecting the Ubuntu image version:


```
From: ubuntu:{{ VERSION }}
```
This specifies that we will be using `ubuntu:22.04`.

### `%setup`

Setup commands are first executed outside of the container on the host system before starting to build the image.

For example, suppose we want to compress some files that will later be added to the container:

```
%setup

    tar -zcvf files.tar.gz ./*.txt
```

This command would compress all the files ending in `.txt` in the current directory into `files.tar.gz` (also in the current directory).


### `%files`

This is where we can specify files to be copied into the container.
```
%files
    files.tar.gz /opt
```
Here, we are copying the `files.tar.gz` that was created in the `%setup` into the `/opt` directory of the image (`/opt/files.tar.gz`).

### `%environment`

Here we specify environmental variables that we want set within the container.

```
%enviroment
    export PATH=$PATH:/app/bin
    export DEFAULT_PORT=8001
```

In this example, we set two environmental variables. First, we modify the `PATH` to include `/app/bin`, where the hypothetical binaries for our application reside. Second, we specify the `DEFAULT_PORT` to be `8001`.

We can access these variables anytime within the container or the build process.

### `%post`

In this section, we specify the command we want to run after the base image has downloaded. Environmental variables for the host system are not passed, so this can be considered a clean environment.

This will likely be the most detailed section of your definition script. For example:

```
%post
    apt-get update && apt-get install -y gcc
    pip install ipython
```

In the above example, we are simply updating the Ubuntu base image and installing `gcc`. We then install `ipython` using `pip`.

This is a simple example, but `%post` would be the section where dependencies would be installed and/or compiled.


### `%runscript`

This is where we define a set of commands that will be executed when running `apptainer run image.sif` or when running the image itself as a command (e.g., `./image.sif`).

Internally, these commands will form a simple script that will be executed.

```
%runscript
    ipython
```

This example will start an IPython interpreter. We could have something more complicated, such as:

```
%runscript
    echo "Recieved the following arguements $*"
    ipython $*
```

This will output the arguments passed before executing them with IPython. For example:

```
> apptainer run ./jupyter.sif --version
Recieved the following arguements --version
8.22.2
```
Here, we're passing `--version` as an argument. This gets passed and run as `ipython --version`, which gives `8.22.2`.

One could use the `%runscript` section to define a default behavior and how arguments are handled.



### `%startscript`

This is similar to the `%runscript` section where we create a script to be run when running the container. Specifically, the `%startscript` runs when the container is launched as an `instance` rather than a process launched with `run` or `exec`. Instances can be considered more of a daemon, which will have a more passive interface. For example, an instance may monitor a port to receive a command that controls its behavior. It might be better to launch a web server as an instance.

Likewise, if you have multiple steps in a data pipeline, they could be passed between instances which are persistent compared to the analysis target.




### `%test`

This defines a test script that is run at the end of the build process and can be used to ensure the validity of the built container.

For example, if we are building a data pipeline, we might want to make sure we get the expected answer.


```
%test
    python test_script.py
    if [ $? -eq 0 ]; then
        echo "Script executed successfully"
    else
        echo "Script failed"
        exit 1
    fi
```
Here we are running `test_script.py`. The output of this code will be accessible using `$?`, which returns the last return code.

```
    if [ $? -eq 0 ]; then
```
This line checks if the return code is 0, which is a typical code for a successful execution. In our Python code, we would have a line like:

```
if successful_test:
    exit(0)
else:
    exit(1)
```
If the code executes successfully, then the return will be 0; otherwise, it will be 1.


### `%labels`
```
%labels
    Author myuser@example.com
    Version v0.0.1
    MyLabel Hello World
```

Here we define a set of labels that are viewable using the `apptainer inspect` command.

Versioning can be super important when developing an application. Maintaining an up-to-date version number can prevent a lot of headaches when trying to debug issues.


### `%help`

Help specifies a help message that will be outputted:

```
%help
    This is a container with jupyter lab and notebook install
```

This can be accessed using:
```
apptainer run-help my_container.sif
```


## Example definition script
Here is an example of a `.def` file which installs `Jupyter`, `IPython`, `Matplotlib`, and `NumPy`.
```
Bootstrap: docker
From: python:latest

%post
    pip install jupyter ipykernel jupyterlab notebook
    pip install matplotlib numpy

%environment
    export DEFAULT_PORT=8001

%runscript
    ipython $*

%startscript
    jupyter lab --port=$DEFAULT_PORT
```

This can be built with:
```
> apptainer build jupyter.sif jupyter.def
```

The `runscript` will take arguments and pass them to IPython. For example:
```
> ./jupyter.sif hello.py
Hello, world!
Inside of container!
```

The `startscript` will start a Jupyter Lab on port 8001. This can be launched using:
```
> apptainer instance start jupyter.sif jupyter-server 
```

When navigating to `http://localhost:8001`, we'll notice that we need to log in. We can get a login code using:"
```
> apptainer exec instance://jupyter-server jupyter lab list
Currently running servers:
http://localhost:8001/?token=643b97dc15207ca577782ea2e03a3ec1f9337a4445bc1db8 :: /home/obriens/Documents/apptainer
```

Clicking on that link will log us in. We need to remember to `stop` the instance once we're finished.

```
> apptainer instance stop jupyter-server            
```

## Example of a multi-stage build

As mentioned earlier, using multi-stage builds can help decrease the final size of the `sif` file.

Consider the following `C++` code:

```c++ title="convert_units.cpp"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char *argv[]){

    // parse the command passed
    // Input is in meters
    float input = atof(argv[1]);

    // convert unit to mm
    float output = input * 1e3;

    // output to a text file
    ofstream out_file;
    out_file.open("test.txt");
    out_file << output << endl;

    // Also print
    cout << output << endl;
    return 0;
}
```

This will convert meters to mm. We can imagine this being part of a larger data analysis pipeline.

This can be compiled using:
```
g++ convert_units.cpp -o convert_units
```
This will create a binary called `convert_units`.

Let's start to build the definition file:

```title="single_stage.def" linenums="1"
Bootstrap: docker
From: ubuntu
Stage: build

%files
    convert_units.cpp /build/convert_units.cpp
    
%post
    apt-get update && apt-get upgrade -y && apt-get install -y g++
    g++ /build/convert_units.cpp -o /bin/convert_units

%runscript
    /bin/convert_units $*
```

Here we have a single stage called `build`. In this stage, we copy the source code to the `/build` directory at the `%files` stage. In the `%post` stage, we update the OS and install `g++`, a C++ compiler. We then compile the code to `/bin/convert_units`. We then specify this as the entry point of the `%runscript` stage.

We can run this as:
```
> ./single_stage.sif 1.25
1250
```

You'll notice that the `convert_units.cpp` file is no longer needed once `convert_units` is compiled. Likewise, we only need `g++` to compile `convert_units`; we don't use it later in the file. We could turn this into a multi-stage build:

```linenums="1" title="multi_stage.def" hl_lines="17"
Bootstrap: docker
From: ubuntu
Stage: build


%files
    convert_units.cpp /build/convert_units.cpp
    
%post
    apt-get update && apt-get upgrade -y && apt-get install -y g++
    g++ /build/convert_units.cpp -o /build/convert_units

Bootstrap: docker
From: ubuntu
Stage: final

%files from build
  /build/convert_units /bin/convert_units

%post
    apt-get update && apt-get upgrade -y

%runscript
    /bin/convert_units $*
```

The definition file is similar to the `single_stage.def` file; however, we have broken this up into two stages. 

The first stage, tagged as `build`, will add the source file `convert_units.cpp` to the image, update the OS, install `g++`, and compile `/build/convert_units`.

The second stage, called `final`, uses the same `Bootstrap` and base image (`ubuntu`) as the `build` stage. However, at the `%files` stage on line 17, we are only copying the `/build/convert_units` from the `build` stage to `/bin/convert_units` in the `final` stage. We still want to make sure we have an up-to-date OS (security updates are always important), so we still run `apt-get update && apt-get upgrade -y`. Finally, the `%runscript` stage is only included in the `final` stage.

We can see that we get the same behavior from both images:

```
> ./single_stage.sif 1.25 ;  ./multi_stage.sif 1.25
1250
1250
```

However, when we look at the size of the files, we see a difference:
```
> ls -lah ./*_stage.sif
-rwxr-xr-x 1 obriens obriens  63M Mar 25 14:36 ./multi_stage.sif
-rwxr-xr-x 1 obriens obriens 142M Mar 25 14:36 ./single_stage.sif
```


You'll notice that the `multi_stage.sif` build is around half the size of `single_stage.sif`. This is partly due to the `multi_stage.sif` not containing the source code (`convert_units.cpp`), but also due to it not containing `g++`.


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

