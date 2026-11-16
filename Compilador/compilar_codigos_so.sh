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
generated_files=(
    ast.txt
    symbol_table.txt
    intermediate_code.txt
    assembly_code.txt
    quadruples_assembly_code.txt
    binary.txt
)

for source_dir in "$SOURCE_ROOT"/code[0-9]*_*; do
    [[ -d "$source_dir" ]] || continue

    code_name="$(basename -- "$source_dir")"
    source_file="$source_dir/$code_name.txt"

    if [[ ! -f "$source_file" ]]; then
        printf 'Erro: código-fonte não encontrado: %s\n' "$source_file" >&2
        exit 1
    fi

    printf 'Compilando %s...\n' "$code_name"
    for generated_file in "${generated_files[@]}"; do
        rm -f -- "$WORK_DIR/output/$generated_file"
    done

    if ! (cd -- "$WORK_DIR" && "$COMPILER" "$source_file"); then
        printf 'Erro ao compilar %s\n' "$source_file" >&2
        exit 1
    fi

    for generated_file in "${generated_files[@]}"; do
        generated_path="$WORK_DIR/output/$generated_file"
        if [[ ! -f "$generated_path" ]]; then
            printf 'Erro: compilação de %s não gerou output/%s\n' \
                "$code_name" "$generated_file" >&2
            exit 1
        fi

        cp -- "$generated_path" "$source_dir/$generated_file"
    done

    ((compiled_count += 1))
done

if (( compiled_count == 0 )); then
    printf 'Erro: nenhum código encontrado em %s\n' "$SOURCE_ROOT" >&2
    exit 1
fi

printf 'Concluído: %d código(s) compilado(s).\n' "$compiled_count"

combined_file="$WORK_DIR/all_codes_binary.txt"
: > "$combined_file"
combined_count=0

for source_dir in "$SOURCE_ROOT"/code[0-9]*_*; do
    [[ -d "$source_dir" ]] || continue

    code_name="$(basename -- "$source_dir")"
    binary_file="$source_dir/binary.txt"
    padded_file="$WORK_DIR/padded_binary.txt"
    instruction_count=0
    : > "$padded_file"

    while IFS= read -r instruction || [[ -n "$instruction" ]]; do
        if [[ ! "$instruction" =~ ^[01]{32}$ ]]; then
            printf 'Erro: instrução binária inválida em %s\n' "$binary_file" >&2
            exit 1
        fi

        if (( instruction_count >= 512 )); then
            printf 'Erro: %s excede o limite de 512 instruções\n' "$code_name" >&2
            exit 1
        fi

        if (( instruction_count > 0 )); then
            printf '\n' >> "$padded_file"
        fi
        printf '%s' "$instruction" >> "$padded_file"
        ((instruction_count += 1))
    done < "$binary_file"

    if (( instruction_count == 0 )); then
        printf 'Erro: arquivo binário vazio: %s\n' "$binary_file" >&2
        exit 1
    fi

    while (( instruction_count < 360 )); do
        printf '\n%032d' 0 >> "$padded_file"
        ((instruction_count += 1))
    done

    if (( combined_count > 0 )); then
        printf '\n' >> "$combined_file"
    fi
    cat -- "$padded_file" >> "$combined_file"
    ((combined_count += 1))
done

cp -- "$combined_file" "$SOURCE_ROOT/all_codes_binary.txt"
printf 'Arquivo combinado gerado: %s/all_codes_binary.txt (%d códigos, 512 instruções cada).\n' \
    "$SOURCE_ROOT" "$combined_count"
