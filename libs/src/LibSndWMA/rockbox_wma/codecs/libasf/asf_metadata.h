#pragma once

void asf_metadata_init(struct mp3entry* id3, asf_waveformatex_t* wfx);
bool asf_metadata_get(struct mp3entry* id3, asf_waveformatex_t* wfx);
void asf_metadata_free(struct mp3entry* id3, asf_waveformatex_t* wfx);

