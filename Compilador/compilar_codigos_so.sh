#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
COMPILER="$SCRIPT_DIR/compilador"
SOURCE_ROOT="$SCRIPT_DIR/output/Codigos_SO"

if [[ ! -x "$COMPILER" ]]; then
    printf 'Erro: compilador não encontrado ou não executável: %s\n' "$COMPILER" >&2
    exit 1
fi

if [[ ! -d "$SOURCE_ROOT" ]]; then
    printf 'Erro: diretório de códigos não encontrado: %s\n' "$SOURCE_ROOT" >&2
    exit 1
fi

WORK_DIR="$(mktemp -d)"
trap 'rm -rf -- "$WORK_DIR"' EXIT

shopt -s nullglob
compiled_count=0

for source_dir in "$SOURCE_ROOT"/code[0-9]*_*; do
    [[ -d "$source_dir" ]] || continue

    code_name="$(basename -- "$source_dir")"
    source_file="$source_dir/$code_name.txt"
    destination="$source_dir/binary.txt"

    if [[ ! -f "$source_file" ]]; then
        printf 'Erro: código-fonte não encontrado: %s\n' "$source_file" >&2
        exit 1
    fi

    printf 'Compilando %s...\n' "$code_name"
    rm -f -- "$WORK_DIR/output/binary.txt"
    if ! (cd -- "$WORK_DIR" && "$COMPILER" "$source_file"); then
        printf 'Erro ao compilar %s\n' "$source_file" >&2
        exit 1
    fi

    if [[ ! -f "$WORK_DIR/output/binary.txt" ]]; then
        printf 'Erro: compilação de %s não gerou output/binary.txt\n' "$code_name" >&2
        exit 1
    fi

    cp -- "$WORK_DIR/output/binary.txt" "$destination"
    ((compiled_count += 1))
done

if (( compiled_count == 0 )); then
    printf 'Erro: nenhum código encontrado em %s\n' "$SOURCE_ROOT" >&2
    exit 1
fi

printf 'Concluído: %d código(s) compilado(s).\n' "$compiled_count"
