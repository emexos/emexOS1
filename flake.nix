{
  description = "emexOS dev flake";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = inputs: inputs.flake-utils.lib.eachDefaultSystem(system: let
    pkgs = import inputs.nixpkgs;
  in {
    devShells.default = pkgs.mkShellNoCC {
      nativeBuildInputs = with pkgs; [
        # Toolchain
        pkgsCross.x86-embedded.stdenv.cc
        nasm
        make

        # For fetching dependencies
        git
      ];
    };
  });
}
