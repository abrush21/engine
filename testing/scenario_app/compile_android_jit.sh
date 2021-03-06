#!/bin/bash
# Copyright 2013 The Flutter Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

HOST_TOOLS=$1
DEVICE_TOOLS=$2

if [[ ! -d "$HOST_TOOLS" ]]; then
  echo "Must specify the host out directory containing dart."
  exit 1
fi

if [[ ! -d "$DEVICE_TOOLS" ]]; then
  echo "Must specify the device out directory containing gen_snapshot."
  exit 1
fi

PUB_VERSION=$($HOST_TOOLS/dart-sdk/bin/pub --version)
echo "Using Pub ${PUB_VERSION} from $HOST_TOOLS/dart-sdk/bin/pub"

$HOST_TOOLS/dart-sdk/bin/pub get

echo "Using dart from $HOST_TOOLS, gen_snapshot from $DEVICE_TOOLS."

OUTDIR="${BASH_SOURCE%/*}/build/app"
FLUTTER_ASSETS_DIR=$OUTDIR/assets/flutter_assets
LIBS_DIR="${BASH_SOURCE%/*}/android/app/libs"

echo "Creating directories..."

mkdir -p $OUTDIR
mkdir -p $FLUTTER_ASSETS_DIR
mkdir -p $LIBS_DIR

echo "Compiling kernel..."

"$HOST_TOOLS/dart" \
  "$HOST_TOOLS/gen/frontend_server.dart.snapshot" \
  --sdk-root "$HOST_TOOLS/flutter_patched_sdk" \
  --target=flutter \
  --no-link-platform \
  --output-dill "$FLUTTER_ASSETS_DIR/kernel_blob.bin" \
  "${BASH_SOURCE%/*}/lib/main.dart"

echo "Compiling JIT Snapshot..."

"$DEVICE_TOOLS/gen_snapshot" --deterministic \
  --enable-asserts \
  --no-causal_async_stacks \
  --lazy_async_stacks \
  --isolate_snapshot_instructions="$OUTDIR/isolate_snapshot_instr" \
  --snapshot_kind=app-jit \
  --load_vm_snapshot_data="$DEVICE_TOOLS/../gen/flutter/lib/snapshot/vm_isolate_snapshot.bin" \
  --load_isolate_snapshot_data="$DEVICE_TOOLS/../gen/flutter/lib/snapshot/isolate_snapshot.bin" \
  --isolate_snapshot_data="$FLUTTER_ASSETS_DIR/isolate_snapshot_data" \
  --isolate_snapshot_instructions="$FLUTTER_ASSETS_DIR/isolate_snapshot_instr" \
  "$FLUTTER_ASSETS_DIR/kernel_blob.bin"

cp "$DEVICE_TOOLS/../flutter.jar" "$LIBS_DIR"

echo "Created $OUTDIR."
