QT_VERSION="5.15.17"
QT5_STATIC_IMAGE_TAG="qtweb-qt5-static-poc:${QT_VERSION}"

format_duration() {
    local total_seconds="$1"
    local hours minutes seconds

    if (( total_seconds < 0 )); then
        total_seconds=0
    fi

    hours=$((total_seconds / 3600))
    minutes=$(((total_seconds % 3600) / 60))
    seconds=$((total_seconds % 60))

    if (( hours > 0 )); then
        printf '%dh %02dm %02ds' "$hours" "$minutes" "$seconds"
        return
    fi

    if (( minutes > 0 )); then
        printf '%dm %02ds' "$minutes" "$seconds"
        return
    fi

    printf '%ds' "$seconds"
}
