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

          # ESPHome copies the component into its build dir before compiling, so the
          # generated compile_commands.json points at those copies. Rewrite the paths to
          # the checkout so clangd picks up the real flags for the files being edited.
          gen-compile-db = pkgs.writeShellApplication {
            name = "gen-compile-db";
            runtimeInputs = with pkgs; [
              esphome
              git
              gnused
              findutils
              coreutils
            ];
            text = ''
              root=$(git rev-parse --show-toplevel)
              config=$(realpath "''${1:-$root/tests/test.yaml}")

              # compile_commands.json is only written by a real ESP-IDF build
              esphome compile "$config"

              db=$(find "$(dirname "$config")/.esphome/build" -path '*/build/compile_commands.json' \
                -not -path '*/bootloader/*' -printf '%T@ %p\n' | sort -n | tail -n1 | cut -d' ' -f2-)
              build_src=$(dirname "$(dirname "$db")")/src/esphome/components/vs481b

              sed "s|$build_src|$root/components/vs481b|g" "$db" > "$root/compile_commands.json"
              echo "Wrote $root/compile_commands.json (from $db)"
            '';
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
              gen-compile-db
            ];
          };

          packages.gen-compile-db = gen-compile-db;

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
                # invalid_*.yaml must fail validation with the message in their "# expect:" line
                for config in tests/*.yaml; do
                  echo "Checking $config"
                  case "$config" in
                    tests/invalid_*)
                      expected=$(sed -n 's/^# expect: //p' "$config")
                      if esphome config "$config" > out.log 2>&1; then
                        echo "$config: expected validation to fail"
                        exit 1
                      fi
                      if ! grep -qF "$expected" out.log; then
                        echo "$config: expected error not found: $expected"
                        cat out.log
                        exit 1
                      fi
                      ;;
                    *)
                      esphome config "$config" > /dev/null
                      ;;
                  esac
                done
                touch $out
              '';
        };
    };
}
