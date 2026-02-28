{
  description = "emexos dev flake";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default";
  };
  outputs = inputs: let
    forEachSystem = inputs.nixpkgs.lib.genAttrs (import inputs.systems);
    pkgs = forEachSystem (system: import inputs.nixpkgs {inherit system;});
  in {
    formatter = forEachSystem (system: pkgs.${system}.alejandra);
    devShells = forEachSystem (system: {
      default = pkgs.${system}.mkShellNoCC {
        nativeBuildInputs = with pkgs.${system}; [
					# Compiler and assembler
          pkgsCross.x86_64-embedded.stdenv.cc
          nasm

					# Provides a lot of handy tools for C/C++ dev e.g. formatter (clang-format)
					clang-tools

					# For generating a `compile_commands.json` which can be used by clangd
					# That file is used by the clangd lsp to figure out includes and all
					compiledb

					# Provides `xorriso` for creating the emexOS ISO
          libisoburn

					# Git is required to fetch some dependencies
          git
        ];
      };
    });
  };
}