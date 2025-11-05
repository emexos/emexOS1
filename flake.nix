{
  description = "emexos dev flake";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = inputs: inputs.flake-utils.lib.eachDefaultSystem(system: let
    pkgs = import inputs.nixpkgs {inherit system;};
  in {
    devShells.default = pkgs.mkShellNoCC {
      nativeBuildInputs = with pkgs; [
        pkgsCross.x86_64-embedded.stdenv.cc
        nasm
        libisoburn
        git
      ];
    };
  });
}
