{
  description = "ESPHome external component for ATEN VS481B HDMI switches";

  inputs = {
    nixpkgs.url = "https://channels.nixos.org/nixpkgs-unstable/nixexprs.tar.zst";
    systems.url = "github:nix-systems/default";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    git-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = import inputs.systems;

      imports = [
        inputs.treefmt-nix.flakeModule
        inputs.git-hooks.flakeModule
      ];

      perSystem =
        {
          config,
          pkgs,
          lib,
          system,
          ...
        }:
        let
          python = pkgs.python3.withPackages (ps: [ (ps.toPythonModule pkgs.esphome) ]);

          vscode = pkgs.vscode-with-extensions.override {
            vscodeExtensions =
              (with pkgs.vscode-extensions; [
                ms-python.python
                ms-python.vscode-pylance
                charliermarsh.ruff
                llvm-vs-code-extensions.vscode-clangd
                jnoortheen.nix-ide
                redhat.vscode-yaml
                mkhl.direnv
              ])
              ++ pkgs.vscode-utils.extensionsFromVscodeMarketplace [
                {
                  publisher = "esphome";
                  name = "esphome-vscode";
                  version = "2026.4.0";
                  hash = "sha256-6BkJ+zusrjZEz0Xb9mrpHxVxyu6YAwlbFxm9zQvIf+Y=";
                }
              ];
          };
        in
        {
          _module.args.pkgs = import inputs.nixpkgs {
            inherit system;
            config.allowUnfreePredicate =
              pkg:
              builtins.elem (lib.getName pkg) [
                "vscode"
                "vscode-with-extensions"
                "vscode-extension-MS-python-vscode-pylance"
              ];
          };

          treefmt = {
            projectRootFile = "flake.nix";
            programs = {
              nixfmt.enable = true;
              ruff-format.enable = true;
              ruff-check.enable = true;
              clang-format.enable = true;
            };
            
            flakeCheck = false;
          };

          pre-commit = {
            check.enable = false;
            settings.hooks.treefmt = {
              enable = true;
              package = config.treefmt.build.wrapper;
            };
          };

          devShells.default = pkgs.mkShell {
            inputsFrom = [
              config.treefmt.build.devShell
              config.pre-commit.devShell
            ];
            packages = with pkgs; [
              python
              clang-tools
              vscode
              nixd
            ];
          };

          checks.config =
            pkgs.runCommand "vs481b-esphome-config"
              {
                nativeBuildInputs = [ pkgs.esphome ];
              }
              ''
                export HOME=$TMPDIR
                cp -r ${./components} components
                cp -r ${./tests} tests
                chmod -R u+w .
                esphome config tests/test.yaml > /dev/null
                touch $out
              '';
        };
    };
}
