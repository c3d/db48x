#!/bin/bash
# Remove macOS quarantine attribute to allow the downloaded app to run
# This is needed because macOS marks downloaded files as potentially unsafe

echo "Removing quarantine attribute from db48x.app..."
xattr -dr com.apple.quarantine db48x.app

if [ $? -eq 0 ]; then
    echo "✓ db48x.app is now allowed to run"
    echo "You can now open db48x.app by double-clicking it"
else
    echo "⚠ Failed to remove quarantine attribute"
    echo "You may need to run: sudo xattr -dr com.apple.quarantine db48x.app"
fi

