# re-github-stargazers

This is a simple [ReasonML](https://reasonml.github.io/) application which
displays information about an organization's Github repositories and their
stargazers.

It was written primarily as en exercise to familiarise myself with the language
and libraries.

## Getting started

This should be buildable in any platform that can build the patched Ocaml compiler
BuckleScript needs. If in doubt, you can build the development Docker container
declared [here](docker-dev/Dockerfile).

If using Nix, you can install the compiler using [this derivation](nix/ocaml-bs.nix):

```sh
nix-env -iA ocaml-bs -f nix/release.nix
```

You will also need to install `binutils` and `gnumake`

### Run Project

```sh
yarn install
yarn start
# in another tab
yarn run webpack
```

After you see the webpack compilation succeed (the `yarn run webpack` step), open up `build/index.html` (**no server needed!**). Then modify whichever `.re` file in `src` and refresh the page to see the changes.

### Run tests

```
yarn test
```

### Build for Production

```sh
yarn run build
yarn run webpack:production
```

This will replace the development artifact `build/index.js` for an optimized version.

## TODO

- [x] List all GitHub public repositories from an organization
- [x] For each repository display:
    - [x] Repo full name, with link to the actual repo
    - [x] License info
        - [x] Embedded license info from repositories API response
        - [ ] Full license info from `license_url` endpoint?
    - [x] Number of stargazers
- [x] Drill down to a repository's 20 first stargazers in a different panel
- [x] Display error message
    - [x] If API is down
    - [x] If organization is not found
- [x] Make page responsive
- [x] Bundle application with Webpack
- [ ] Unit tests
    - [x] Rendering tests
    - [ ] reducer tests
