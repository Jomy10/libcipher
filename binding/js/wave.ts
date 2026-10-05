export enum AudioFormat {
  PCM
};

function audioFormatToNumber(format: AudioFormat) {
  switch (format) {
    case AudioFormat.PCM: return 1;
  }
}

export default class WaveFile {
  audioFormat: number;
  numChannels: number;
  sampleRate: number;
  bitsPerSample: number;
  byteRate: number;
  blockAlign: number;
  audioData: Uint8Array;

  constructor(
    audioFormat: AudioFormat,
    numChannels: number,
    sampleRate: number,
    bitsPerSample: number,
    audioData: Uint8Array | null
  ) {
    this.audioFormat = audioFormatToNumber(audioFormat);
    this.numChannels = numChannels
    this.sampleRate = sampleRate;
    this.bitsPerSample = bitsPerSample;
    this.byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    this.blockAlign = numChannels * (bitsPerSample / 8);
    this.audioData = audioData == null ? new Uint8Array() : audioData;
  }

  setBytes(bytes: Uint8Array) {
    this.audioData = bytes;
  }

  addBytes(bytes: Uint8Array) {
    const oldData = this.audioData;
    this.audioData = new Uint8Array(this.audioData.length + bytes.length);
    this.audioData.set(oldData);
    this.audioData.set(bytes, oldData.length);
  }

  static fileToData(fileData: Uint8Array): Uint8Array {
    const encoder = new TextEncoder();
    const view = new DataView(fileData.buffer, fileData.byteOffset, fileData.byteLength);

    if (
      view.getUint32(0, false) !== 0x52494646 || // "RIFF"
      view.getUint32(8, false) !== 0x57415645    // "WAVE"
    ) throw new Error("Not a valid wave file");

    let offset = 12;

    while (offset + 8 <= fileData.byteLength) {
      const chunkId = view.getUint32(offset, false);
      const chunkSize = view.getUint32(offset + 4, true);

      if (chunkId == 0x64617461 /* data */) {
        const start = offset + 8;
        const end = start + chunkSize;

        if (end > fileData.byteLength)
          throw new Error("Truncated wave file");

        return fileData.subarray(start, end);
      }

      offset += 8 + chunkSize + (chunkSize & 1);
    }

    throw new Error("wave data chunk not found");
  }

  addFileData(fileData: Uint8Array) {
    this.addBytes(WaveFile.fileToData(fileData));
  }

  getBytes(): Uint8Array {
    return this.audioData;
  }

  getBlockAlign(): number {
    return this.blockAlign;
  }

  toBytes(): Uint8Array {
    const encoder = new TextEncoder();

    const chunkId = "RIFF";
    const format = "WAVE";

    const subchunk1Size = 16;
    const subchunk1Id = "fmt ";

    const subchunk2Id = "data";
    const numBytesInData = this.audioData.length;
    const numSamples = numBytesInData / (2 * this.numChannels);
    const subchunk2Size = numSamples * this.numChannels * (this.bitsPerSample / 8);

    const chunkSize = 4 + (8 + subchunk1Size) + (8 + subchunk2Size);

    let output = new Uint8Array(chunkSize + 8 /* RIFF + SIZE */);

    let dv = new DataView(output.buffer);

    let idx = 0;
    // head
    output.set(encoder.encode(chunkId), idx);
    idx += chunkId.length;

    dv.setUint32(idx, chunkSize, true);
    idx += 4;

    output.set(encoder.encode(format), idx);
    idx += format.length

    // subchunk 1
    output.set(encoder.encode(subchunk1Id), idx);
    idx += subchunk1Id.length;

    dv.setUint32(idx, subchunk1Size, true);
    idx += 4;

    dv.setUint16(idx, this.audioFormat, true);
    idx += 2;

    dv.setUint16(idx, this.numChannels, true);
    idx += 2;

    dv.setUint32(idx, this.sampleRate, true);
    idx += 4;

    dv.setUint32(idx, this.byteRate, true);
    idx += 4;

    dv.setUint16(idx, this.blockAlign, true);
    idx += 2;

    dv.setUint16(idx, this.bitsPerSample, true);
    idx += 2;

    // Subchunk 2
    output.set(encoder.encode(subchunk2Id), idx);
    idx += subchunk2Id.length;

    dv.setUint32(idx, subchunk2Size, true);
    idx += 4;

    output.set(this.audioData, idx);

    return output;
  }
};
