# Releasing firmware

A GitHub repository for Twomes firmware comes with [a workflow that automatically initiates actions to build and create a draft for a new release](https://github.com/energietransitie/twomes-generic-esp-firmware/blob/main/.github/workflows/release.yml). 

Such a draft should be verified before it is published. At the time of verifying, a title and description can be added to the release.

## Semantic versioning
Always use a [semantic version number](https://semver.org) when following the steps below! If you create a tag which is not a semantic version number prepended with "v" (e.g. "v1.0.0", "v1.3.12" or "v12.34.2"), Over-The-Air firmare updates will not work properly.

## Initiating a build and draft release
Follow the steps below to start the GitHub actions workflow which build the binaries and creates a draf release.

### Prerequisites

- [Git](https://git-scm.com/downloads)
- [Git GPG signing](https://docs.github.com/en/authentication/managing-commit-signature-verification/generating-a-new-gpg-key) (recommended)
- [Singing key secret set up in GitHub Actions](secure-boot-firmware.md#github-actions-workflow)

### Steps
1. If you have not already cloned the repository, do this now:
    ```shell
    git clone https://github.com/energietransitie/twomes-generic-esp-firmware.git
    ```
2. If you are not already on the main branch, checkout to the main branch:
    ```shell
    git checkout main
    ```
3. Make sure your main branch is up-to-date with the remote (typically origin):
    ```shell
    git pull origin main
    ```
4. Create a new tag with the new [semantic version number](#semantic-versioning):
    ```shell
    git tag -s v<semantic version> -m "<message>"
    ```
    > The message can contain a description of the tag.
5. Upload the created tag to the remote (typically origin):
    ```shell
    git push origin <tag name>
    ```

## Publishing
> This information below only applies to firwmare released by the [Research Group Energy Transition](https://github.com/energietransitie).

1. Go to the [releases page](https://github.com/energietransitie/twomes-generic-esp-firmware/releases).
2. The latest draft release should be at the top of the page. Click on the pen icon in the top right of the release.
3. You can now change the release title and description.
4. The associated tag can be seen. Check if this is correct.
5. The attached binaries can be viewed. Check if these are correct. There should be an archive `<repository_name>_<version>.zip` and a `firmware.bin`.
6. Click on `Publish release` to release the firmware version.
