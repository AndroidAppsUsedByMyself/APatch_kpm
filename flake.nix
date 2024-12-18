{
  description = "A multi-platform flake for compiling .kpm files with LLVM dependency and devshell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        localSystem = system;
        crossSystem = {
          config = "aarch64-unknown-linux-gnu";
        };
      };

      # List directories excluding "KernelPatch" and ensuring they have a Makefile
      listKpmDirs =
        builtins.filter
        (dir: dir != "KernelPatch" && builtins.pathExists (self + "/" + dir + "/Makefile"))
        (builtins.attrNames (builtins.readDir self));

      # Generate individual KPM module packages
      modulePackages = builtins.listToAttrs (builtins.map (folder: {
          name = folder;
          value = pkgs.stdenv.mkDerivation rec {
            pname = folder;
            version = "1.0";

            src = builtins.path {
              path = self;
            };

            buildInputs = [pkgs.llvm pkgs.clang pkgs.git];

            buildPhase = ''
              set -e
              mkdir -p $out/kpm_packages/${folder}
              cd $out/kpm_packages/${folder}
              make -C ${src}/${folder}
            '';

            installPhase = ''
            '';

            meta = {
              description = "Compiled KPM module for ${folder}";
              platforms = pkgs.lib.platforms.linux;
            };
          };
        })
        listKpmDirs);

      # Combined package for all KPM modules
      allKpmPackage = pkgs.stdenv.mkDerivation {
        pname = "all-kpm";
        version = "1.0";

        nativeBuildInputs = builtins.attrValues modulePackages;

        buildPhase = ''
          mkdir -p $out/kpm_packages
          for dir in ${toString listKpmDirs}; do
          done
        '';

        meta = {
          description = "Combined package of all KPM modules";
          platforms = pkgs.lib.platforms.linux;
        };
      };

      # Development shell with LLVM and tools
      devShell = pkgs.mkShell {
        buildInputs = [pkgs.llvm pkgs.clang pkgs.git];
        shellHook = ''
          echo "Welcome to the KPM DevShell! LLVM, Clang, and build tools are available."
        '';
      };
    in {
      # Individual packages for each KPM module
      packages =
        builtins.listToAttrs (builtins.map (folder: {
            name = folder;
            value = pkgs.lib.getAttr folder modulePackages;
          })
          listKpmDirs)
        // {"all-kpm" = allKpmPackage;};

      devShell = devShell;
    });
}
