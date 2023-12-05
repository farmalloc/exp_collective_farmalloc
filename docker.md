Docker-based usage
===

## How to Build

```bash
docker build -t collective_farmalloc:latest .
```

## How to Run

```bash
sudo sysctl vm.unprivileged_userfaultfd=1
docker run -it \
  --security-opt seccomp=./docker_seccomp.json \
  collective_farmalloc:latest
```

`userfaultfd` システムコールを呼べるようにするために、カーネルとコンテナランタイムそれぞれでパラメータの設定が必要である。
コンテナランタイムの設定の[デフォルト値](https://github.com/moby/moby/blob/2a38569337f97168792b8c0b5dd606032fe1dcac/profiles/seccomp/default.json)からの差分は、 `syscalls > names` に `userfaultfd` を加えたことだけである。
