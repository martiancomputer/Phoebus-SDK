# Agent guide: Phoebus-SDK

This is the shared SDK used as the `sdk/` submodule by PhoebusBSP-6 and PhoebusBSP-7. It is a separate Git repository: a local SDK edit does not update either BSP until it is committed, published, and each parent gitlink is intentionally advanced. Do not commit the same shared fix only in a disposable BSP build tree.

## Where things live

- `vendor/` holds the shared Realtek platform, Ethernet/FleetConntrack, and wireless source snapshot. Changes here affect both BSPs when their pins advance; keep kernel-version-specific compatibility edits in BSP overlays instead.
- `rootfs/` builds the common initramfs/root filesystem and tracks its shared skeleton.
- `s6/` and `s6-hpd/` provide service supervision and hotplug behavior.
- `net/` contains network policy and configuration; `ap/` contains access-point integration.
- `scripts/` and `tools/` contain SDK build, fetch, image, and diagnostic utilities.
- `tests/` and `docs/` hold tests and public technical documentation. `secrets/`, local provisioning material, flash dumps, logs, captures, and `.env` files require especially careful review and must not carry live credentials into Git.

Read the relevant BSP `build.sh` to understand how a shared file is grafted into its kernel. Test shared changes against both pinned BSPs where practical. Do not mistake a successful compile for a hardware result.

## Privacy and publication rules

Never commit credentials, Wi-Fi secrets, passwords, API keys/tokens, private keys, cookies, authentication headers, private NAND/user-configuration dumps, or live `.env` files. Do not publish personal usernames/emails, hostnames, account IDs, absolute home/workspace paths, device serials, or unnecessary identifying MAC/IP values. Use neutral placeholders (`$HOME`, `<user>`, `<host>`, `<device>`, `<account-id>`) and ignored local configuration. Do not indiscriminately strip subnet addresses required for working TFTP, LAN, or other networking code; review whether the exact address is functional and safe to publish.

Treat scratch notes, debug logs, packet captures, prompts, local provisioning artifacts, and machine-specific instructions as private by default. Add narrow `.gitignore` rules for local-only files. Normal public docs such as `README.md` stay tracked.

Before every commit, review `git diff --cached`, run `git diff --cached --check`, and scan staged additions plus commit metadata for sensitive values. Before every push, inspect all outgoing commits and their metadata. If older history contains a secret or personal identifier, do not assume a new commit removes it; stop, coordinate any necessary rotation and history rewrite, and update both BSP submodule pins afterward. Never force-push without explicit authorization and a verified remote ref. Keep public docs, tests, examples, and commit messages sanitized.
