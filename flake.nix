{
  description = "A multi-platform flake for compiling .kpm files with LLVM dependency and devshell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
    KernelPatch = {
      url = "github:bmax121/KernelPatch/362c0b3f9598a1420a4263e484fb6414bf583896";
      flake = false;
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    KernelPatch,
    ...
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
        (dir: dir != "KernelPatch" && dir != "Utils" && builtins.pathExists (self + "/" + dir + "/Makefile"))
        (builtins.attrNames (builtins.readDir self));

      # Generate individual KPM module packages
      modulePackages = builtins.listToAttrs (builtins.map (folder: {
          name = folder;
          value = pkgs.stdenvNoCC.mkDerivation rec {
            pname = folder;
            version = "1.0";

            src = builtins.path {
              path = self;
            };

            buildInputs = with pkgs; [llvm clang git];

            patchPhase = ''
              mkdir -vp src/KernelPatch
              cp -r ${src}/* ./src
              cp -r ${KernelPatch}/* src/KernelPatch
              ls -R src
            '';

            buildPhase = ''
              set -e
              chmod -R u+w .
              mkdir -vp $out/kpm_packages/${folder}
              make -C src/${folder}
            '';

            installPhase = ''
              find "src/${folder}" -name "*.kpm" -exec sh -c 'echo "Copying file: {}" && cp {} "$out/kpm_packages/${folder}"' \;
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
    in rec {
      # Individual packages for each KPM module
      kpmPackages =
        builtins.listToAttrs (builtins.map (folder: {
            name = folder;
            value = pkgs.lib.getAttr folder modulePackages;
          })
          listKpmDirs)
        // {"all-kpm" = allKpmPackage;};

      packages = kpmPackages;
      devShells = {
        default = devShell;
      };
    });
}
