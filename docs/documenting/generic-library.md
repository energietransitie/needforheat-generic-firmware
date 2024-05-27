# Generic library

In this section we describe how you can contribute to the [documentation of the needforheat-generic-firmware library](https://github.com/energietransitie/needforheat-generic-firmware/tree/main/docs), which is based on [MkDocs](https://www.mkdocs.org/).

## Prerequisites

To facilitate editing, in addition to satisfying the [prerequisites for developing](../starting/prerequisites.md), you should:

* add [GitHub Markdown Preview extionsion for Visual Studio](https://marketplace.visualstudio.com/items?itemName=bierner.github-markdown-preview)
   
## Previewing

To get a full preview of the [MkDocs](https://www.mkdocs.org/) generated content, you should:

* Open a command prompt and enter:
	```shell
	pip install mkdocs-material
	```
    On on Windows: `Windows-key + S`, select `Command Prompt` and type
    ```shell
	py -m pip install mkdocs-material
	```
* change the directory to the root folder of this project and start the server with the command
    ```shell
    mkdocs serve
    ```
* open a web browser on the URL indicated, usually http://127.0.0.1:8000/.

> Should you encounter issues you may try to replace `py -m <command>` in the above commands with:
>
> - `python -m <command>` 
> - `python3 -m <command>`
> - `<command>.py`