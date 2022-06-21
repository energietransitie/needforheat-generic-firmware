# VS Code Devcontainer

## What is this

A devcontainer lets you use a Docker container as a development environment. The container has all the necessary depencies installed and can easily be deleted when it is not needed.

## How to use it

### Prerequisites

You will need the following prerequisites:

-   [Docker desktop](https://docs.docker.com/desktop/) for MacOS or Windows or
    [Docker](https://docs.docker.com/engine/) for Linux
-   [VS Code](https://code.visualstudio.com/)
-   [VS Code remote - containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

### Build and open the devcontainer

1. Open the repository in VS Code.
2. Click on `reopen in container` in the popup that will be displayed in the bottom right.
    > The container has to be built the fist time you run it. This process can take around 10-15 minutes, but is largely dependent on your download speed and computer hardware. After building the container the first time, starting it again will be much faster.

## Notes

This container can be used to build the firmware on any plaftorm (Windows, MacOS or Linux). Windows and MacOS users can not use the devcontainer to deploy binary releases to a device. They should refer to [this](../README.md#deploying) section of the readme to deploy the firmware.

On Linux, the devcontainer can also be used to deploy the firmware to a device. Uncomment the following lines in `.devcontainer.json` to enable this functionality:

```diff
// Uncommend the 6 lines below to use COM-ports (Linux and MacOS only)
- // "mounts": [
- //     "source=/dev,target=/dev,type=bind,consistency=consistent"
- // ],
- // "runArgs": [
- //     "--privileged"
- // ]
+ "mounts": [
+     "source=/dev,target=/dev,type=bind,consistency=consistent"
+ ],
+ "runArgs": [
+     "--privileged"
+ ]
```
