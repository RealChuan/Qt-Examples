#!/bin/bash -ex

delete_file_or_dir() {
    local target=$1

    if [ -e "$target" ]; then
        if [ -d "$target" ]; then
            rm -rf "$target"
            echo "Directory deleted: $target"
        else
            rm "$target"
            echo "File deleted: $target"
        fi
    else
        echo "Error: $target does not exist."
    fi
}

process_plist() {
    local plist_path="$1"

    if [ -z "$plist_path" ]; then
        echo "Error: Plist file path is not provided."
        return 1
    fi

    if [ ! -f "$plist_path" ]; then
        echo "Error: Plist file does not exist at path: $plist_path"
        return 1
    fi

    echo "Converting plist to XML format..."
    plutil -convert xml1 "$plist_path" 2>/dev/null

    echo "Linting plist file..."
    plutil -lint "$plist_path" 2>/dev/null

    if [ $? -eq 0 ]; then
        echo "Plist file processed successfully."
    else
        echo "Error occurred while processing plist file."
        return 2
    fi
}

notarize_app() {
    local target=$1
    if [ ! -f "$target" ]; then
        echo "Error: $target does not exist."
        exit 1
    fi

    xcrun notarytool submit $target --apple-id "***@***" \
        --team-id "******" --password "******" --wait
    xcrun stapler staple $target
}
