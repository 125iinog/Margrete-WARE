#include <windows.h>
#include <cmath>
#include "MargretePlugin.h"
#include "Ware.h"
#include "Utils.h"

MpBoolean Ware::queryInterface(const MpGuid& iid, void** ppobj) {
	if (!ppobj)
		return MP_FALSE;

	if (iid == IID_IMargretePluginBase || iid == IID_IMargretePluginCommand) {
		*ppobj = this;
		addRef();
		return MP_TRUE;
	}
	return MP_FALSE;
}

MpBoolean Ware::getCommandName(wchar_t* text, MpInteger textLength) const {
	wcsncpy_s(text, textLength, L"Margrete-WARE", textLength);
	return MP_TRUE;
}

MpBoolean Ware::invoke(IMargretePluginContext* ctx) {

	ctx->getDocument(Doc.put());
	Doc->getChart(Chart.put());
	Doc->getUndoBuffer(UndoBuffer.put());


	MpInteger currentTick = ctx->getCurrentTick();
	MpInteger notes_count = Chart->getNotesCount();

	std::vector<NoteItem> note_items;
	note_items.resize(notes_count);

	// get original notes
	{
		for (MpInteger i = 0; i < notes_count; i++) {
			Chart->getNote(i, note_items[i].Note.put());
			memset(&note_items[i].Info, 0, sizeof(note_items[i].Info));
			note_items[i].Note->getInfo(&note_items[i].Info);
			note_items[i].MaxWidth = note_items[i].Info.width;
			note_items[i].ChildCount = note_items[i].Note->getChildrenCount();
			if (note_items[i].ChildCount > 0) {
				note_items[i].ChildNote.resize(note_items[i].ChildCount);
				for (MpInteger j = 0; j < note_items[i].ChildCount; j++) {
					GetChild(note_items[i], j, &note_items[i].MaxWidth);
				}
			}
		}
	}

	UndoBuffer->beginRecording();

	// split notes
	{
		MP_NOTEINFO note_info;
		MargreteComPtr<IMargretePluginNote> tap_note, slide_begin, air_crush_begin;


		for (MpInteger i = 0; i < notes_count; i++) {
			MpInteger note_width = note_items[i].MaxWidth, note_x = note_items[i].Info.x;
			MP_NOTEINFO last_note_info;
			memset(&last_note_info, 0, sizeof(last_note_info));

			if (note_width <= 1) continue;
			for (MpInteger j = 0; j < note_width; j++) {
				switch (note_items[i].Info.type) {
				case MP_NOTETYPE_TAP:
				case MP_NOTETYPE_EXTAP:
				case MP_NOTETYPE_FLICK:
				case MP_NOTETYPE_DAMAGE:
				{
					if (j == 0) {
						SetNoteInfo(i);
					}
					else if (Chart->createNote(tap_note.put())) {
						note_info = GetNoteInfo(note_items[i].Info, j, note_x, note_width);

						if (note_info.x == last_note_info.x && note_info.tick == last_note_info.tick && j != 0) {
							note_info.type = MP_NOTETYPE_HOLD;
							note_info.longAttr = MP_NOTELONGATTR_BEGIN;
							tap_note->setInfo(&note_info);
							last_note_info = note_info;

							MargreteComPtr<IMargretePluginNote> slide_child;
							if (Chart->createNote(slide_child.put())) {
								note_info.longAttr = MP_NOTELONGATTR_END;
								note_info.tick += 1;
								slide_child->setInfo(&note_info);

								// air
								AddChild(slide_child, note_items[i], j, note_width, { MP_NOTETYPE_AIR, MP_NOTETYPE_AIRHOLD, MP_NOTETYPE_AIRSLIDE });

								tap_note->appendChild(slide_child.get());
							}

						}
						else {
							tap_note->setInfo(&note_info);
							last_note_info = note_info;

							// air
							AddChild(tap_note, note_items[i], j, note_width, { MP_NOTETYPE_AIR, MP_NOTETYPE_AIRHOLD, MP_NOTETYPE_AIRSLIDE });
						}

						Chart->appendNote(tap_note.get());
					}
					break;
				}
				case MP_NOTETYPE_HOLD:
				case MP_NOTETYPE_SLIDE:
				{
					if (j == 0) {
						SetNoteInfo(i);
					}
					else if (Chart->createNote(slide_begin.put())) {
						// begin
						note_info = GetNoteInfo(note_items[i].Info, j, note_x, note_width);
						slide_begin->setInfo(&note_info);

						// child
						for (MpInteger k = 0; k < note_items[i].ChildCount; k++) {
							MargreteComPtr<IMargretePluginNote> slide_child;

							if (Chart->createNote(slide_child.put()) &&
								(note_items[i].ChildNote[k].Info.type == MP_NOTETYPE_HOLD ||
									note_items[i].ChildNote[k].Info.type == MP_NOTETYPE_SLIDE)) {

								note_info = GetNoteInfo(note_items[i].ChildNote[k].Info, j, note_items[i].ChildNote[k].Info.x, note_width);
								slide_child->setInfo(&note_info);

								// air
								AddChild(slide_child, note_items[i].ChildNote[k], j, note_width, { MP_NOTETYPE_AIR, MP_NOTETYPE_AIRHOLD, MP_NOTETYPE_AIRSLIDE });

								slide_begin->appendChild(slide_child.get());
							}
						}

						Chart->appendNote(slide_begin.get());
					}
					break;
				}
				case MP_NOTETYPE_AIRCRUSH:
				{
					if (j == 0) {
						SetNoteInfo(i);
					}
					else if (Chart->createNote(air_crush_begin.put())) {
						// begin
						note_info = GetNoteInfo(note_items[i].Info, j, note_x, note_width);
						air_crush_begin->setInfo(&note_info);

						// child
						AddChild(air_crush_begin, note_items[i], j, note_width, { MP_NOTETYPE_AIRCRUSH });

						Chart->appendNote(air_crush_begin.get());
					}
					break;
				}
				}
			}
		}
	}

	UndoBuffer->commitRecording();

	ctx->update();

	MessageBoxW((HWND)ctx->getMainWindowHandle(), L"•ˆ–Ê‚ð•ÏŠ·‚µ‚Ü‚µ‚½B", L"Margrete-WARE", MB_ICONINFORMATION);

	return MP_TRUE;
}

void Ware::GetChild(NoteItem& note_item, MpInteger index, MpInteger* max_width) {
	note_item.Note->getChild(index, note_item.ChildNote[index].Note.put());
	memset(&note_item.ChildNote[index].Info, 0, sizeof(note_item.ChildNote[index].Info));
	note_item.ChildNote[index].Note->getInfo(&note_item.ChildNote[index].Info);
	*max_width = *max_width > note_item.ChildNote[index].Info.width ? *max_width : note_item.ChildNote[index].Info.width;
	note_item.ChildNote[index].ChildCount = note_item.ChildNote[index].Note->getChildrenCount();
	if (note_item.ChildNote[index].ChildCount > 0) {
		note_item.ChildNote[index].ChildNote.resize(note_item.ChildNote[index].ChildCount);
		for (MpInteger i = 0; i < note_item.ChildNote[index].ChildCount; i++) {
			GetChild(note_item.ChildNote[index], i, max_width);
		}
	}
}

void Ware::AddChild(MargreteComPtr<IMargretePluginNote>& note, NoteItem& note_item, MpInteger index, MpInteger max_width, std::vector<MpInteger> types) {
	MP_NOTEINFO note_info;
	for (MpInteger i = 0; i < note_item.ChildCount; i++) {
		MargreteComPtr<IMargretePluginNote> child;
		if (Chart->createNote(child.put()) && Utils::IsNoteType(note_item.ChildNote[i].Info.type, types)) {
			note_info = GetNoteInfo(note_item.ChildNote[i].Info, index, note_item.ChildNote[i].Info.x, max_width);
			child->setInfo(&note_info);

			// child
			if (note_item.ChildNote[i].ChildCount > 0) {
				AddChild(child, note_item.ChildNote[i], index, max_width, types);
			}

			note->appendChild(child.get());
		}
	}
}

void Ware::SetNoteInfo(MpInteger index) {
	MargreteComPtr<IMargretePluginNote> note;
	MP_NOTEINFO note_info;
	Chart->getNote(index, note.put());
	note->getInfo(&note_info);
	note_info.width = 1;
	note->setInfo(&note_info);

	MpInteger child_count = note->getChildrenCount();
	if (child_count > 0) {
		SetChildInfo(note, child_count);
	}
}

void Ware::SetChildInfo(MargreteComPtr<IMargretePluginNote> note, MpInteger count) {
	for (MpInteger i = 0; i < count; i++) {
		MargreteComPtr<IMargretePluginNote> child;
		MP_NOTEINFO note_info;
		note->getChild(i, child.put());
		child->getInfo(&note_info);
		note_info.width = 1;
		child->setInfo(&note_info);

		MpInteger child_count = child->getChildrenCount();
		if (child_count > 0) {
			SetChildInfo(child, child_count);
		}
	}
}

MP_NOTEINFO& Ware::GetNoteInfo(const MP_NOTEINFO& note_info, MpInteger index, MpInteger note_x, MpInteger max_width) {
	MP_NOTEINFO info = note_info;
	info.x = GetNoteX(note_x, index, max_width, note_info.width);
	info.width = 1;
	return info;
}

MpInteger Ware::GetNoteX(MpInteger x, MpInteger index, MpInteger max_width, MpInteger width) {
	return x + std::floor(static_cast<double>(index) / (static_cast<double>(max_width) / static_cast<double>(width)));
}