{ pkgsPath ? <nixpkgs>}:
let pkgs = import pkgsPath {};
in
{
  ocaml-bs = pkgs.callPackage ./ocaml-bs.nix {};
}
