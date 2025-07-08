#pragma once
#include <MargretePlugin.h>
#include <vector>

class NoteItem {
public:
	MargreteComPtr<IMargretePluginNote> Note;
	std::vector<NoteItem> ChildNote;
	MP_NOTEINFO Info;
	MpInteger ChildCount = 0;
	MpInteger MaxWidth = 1;
};

class Ware : public IMargretePluginCommand {
public:
	virtual MpInteger addRef() {
		return ++m_refCount;
	}

	virtual MpInteger release() {
		--m_refCount;
		if (m_refCount <= 0) {
			delete this;
			return 0;
		}
		return m_refCount;
	}
	virtual MpBoolean queryInterface(const MpGuid& iid, void** ppobj);

	virtual MpBoolean getCommandName(wchar_t* text, MpInteger textLength) const;
	virtual MpBoolean invoke(IMargretePluginContext* ctx);

	void GetChild(NoteItem& note_item, MpInteger index, MpInteger* max_width);
	void AddChild(MargreteComPtr<IMargretePluginNote>& note, NoteItem& note_item, MpInteger index, MpInteger max_width, std::vector<MpInteger> types);
	void SetNoteInfo(MpInteger index);
	void SetChildInfo(MargreteComPtr<IMargretePluginNote> note, MpInteger count);
	MP_NOTEINFO& GetNoteInfo(const MP_NOTEINFO& note_info, MpInteger index, MpInteger note_x, MpInteger max_width);
	MpInteger GetNoteX(MpInteger x, MpInteger index, MpInteger max_width, MpInteger width);

	MargreteComPtr<IMargretePluginDocument> Doc;
	MargreteComPtr<IMargretePluginChart> Chart;
	MargreteComPtr<IMargretePluginUndoBuffer> UndoBuffer;
private:
	MpInteger m_refCount = 0;
	MP_NOTEINFO _lastNoteInfo;
};