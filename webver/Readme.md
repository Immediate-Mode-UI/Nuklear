Webversion notes
===

## Terminology

**Webversion** (or **webver** for short) is a demonstration of Nuklear being run in the browser via Emscripten.
Currently it supports only the SDL3 backend, but long-term, additional backends may be added.

We use the consistent term **webversion** because it is already referenced across other files and may appear in user-created issues and pull requests.

## Folder Structure

We use the `site` subfolder to store all public webversion files.
Current folder contains **internal** files for webversion support that do not need to be published.

## Update Policy

The webversion is auto-updated through CI/CD workflow scripts. Any push triggers an automated build and deployment process, so no manual steps are required.

## About Logo

The Nuklear logo was suggested by Rafał Jopek and published in [issue #401](https://github.com/Immediate-Mode-UI/Nuklear/issues/401#issuecomment-2066737874)
