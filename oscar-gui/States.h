#ifndef OSCAR_GUI_STATES_H
#define OSCAR_GUI_STATES_H

#include <QObject>

#include <marble/GeoDataLineString.h>
#include <marble/GeoDataCoordinates.h>

#include <memory>

#include <liboscar/StaticOsmCompleter.h>

#include <sserialize/iterator/TransformIterator.h>
#include <sserialize/containers/AbstractArray.h>

#include "SemaphoreLocker.h"

namespace oscar_gui {
	
class LockableState: public QObject {
Q_OBJECT
public:
	LockableState();
	virtual ~LockableState();
public:
	SemaphoreLocker writeLock() const;
	SemaphoreLocker readLock() const;
protected:
	SemaphoreLocker lock(SemaphoreLocker::Type t) const;
private:
	mutable QSemaphore m_sem;
};

class GeometryState: public LockableState {
Q_OBJECT
public:
	typedef enum {DT_INVALID, DT_POINT, DT_RECT, DT_PATH, DT_POLYGON} DataType;
	typedef enum {AT_NONE=0, AT_SHOW=0x1, AT_TRIANGLES=0x2, AT_CELLS=0x4} ActiveType;
public:
	GeometryState() {}
	virtual ~GeometryState() {}
public:
	struct Entry {
		QString name;
		Marble::GeoDataLineString data;
		int active;
		DataType type;
		sserialize::ItemIndex triangles;
		sserialize::ItemIndex cells;
		Entry() : active(AT_NONE), type(DT_INVALID) {}
		Entry(const QString & name, const Marble::GeoDataLineString & data, DataType t) :
		name(name), data(data), active(AT_NONE), type(t)
		{}
	};
	using const_iterator = sserialize::AbstractArrayIterator<const Entry&>;
};

class SearchGeometryState: public GeometryState {
Q_OBJECT
public:
	
public:
	SearchGeometryState();
	virtual ~SearchGeometryState();
public:
	const_iterator begin() const;
	const_iterator end() const;
public:
	void add(const QString & name, const Marble::GeoDataLineString & data, DataType t);
	void remove(std::size_t p);
	void activate(std::size_t p, ActiveType at);
	void deactivate(std::size_t p, ActiveType at);
	void toggle(std::size_t p, ActiveType at);

	void setCells(std::size_t p, const sserialize::ItemIndex & idx);
	void setTriangles(std::size_t p, const sserialize::ItemIndex & idx);
public:
	std::size_t size() const;
	const QString & name(std::size_t p) const;
	ActiveType active(std::size_t p) const;
	DataType type(std::size_t p) const;
	const Marble::GeoDataLineString & data(std::size_t p) const;
	const sserialize::ItemIndex & cells(std::size_t p) const;
	const sserialize::ItemIndex & triangles(std::size_t p) const;
signals:
	void dataChanged(int p);
private:
	std::vector<Entry> m_entries;
};

class TextSearchState: public GeometryState {
Q_OBJECT
public:
	TextSearchState() {}
	virtual ~TextSearchState() {}
public:
	QString searchText() const;
public slots:
	void setSearchText(const QString & value);
signals:
	void searchTextChanged(const QString & value);
private:
	QString m_searchText;
};

class ItemGeometryState: public GeometryState {
Q_OBJECT
public:
	struct GetSecond {
		const Entry & operator()(const std::pair<const uint32_t, Entry> & v) const { return v.second; }
		Entry & operator()(std::pair<const uint32_t, Entry> & v) const { return v.second; }
	};
public:
	ItemGeometryState(const liboscar::Static::OsmKeyValueObjectStore & store);
	virtual ~ItemGeometryState() {}
public:
	const_iterator begin() const;
	const_iterator end() const;
public:
	int active(uint32_t itemId) const;
public slots:
	void clear();
	void activate(uint32_t itemId, ActiveType at);
	void deactivate(uint32_t itemId, ActiveType at);
	void toggleItem(uint32_t itemId, ActiveType at);
signals:
	void zoomToItem(uint32_t itemId);
	void dataChanged();
private:
	///not thread-safe
	void addItem(uint32_t itemId);
private:
	liboscar::Static::OsmKeyValueObjectStore m_store;
	std::unordered_map<uint32_t, Entry> m_entries;
};

class ResultListState: public LockableState {
Q_OBJECT
public:
	ResultListState() {}
	virtual ~ResultListState() {}
public:
	QString queryString() const;
	sserialize::ItemIndex items() const;
	sserialize::CellQueryResult cqr() const;
	uint32_t itemId(uint32_t pos) const;
	std::size_t size() const;
public slots:
	void setResult(const QString & queryString, const sserialize::CellQueryResult & cqr, const sserialize::ItemIndex & items);
signals:
	void dataChanged();
private:
	QString m_qs;
	sserialize::CellQueryResult m_cqr;
	sserialize::ItemIndex m_items;
};

class States: QObject {
Q_OBJECT
public:
	explicit States(const std::shared_ptr<liboscar::Static::OsmCompleter> & cmp);
	States(const States & other) = default;
	States & operator=(const States & other) = default;
public:
	std::shared_ptr<liboscar::Static::OsmCompleter> cmp;
	std::shared_ptr<SearchGeometryState> sgs;
	std::shared_ptr<ItemGeometryState> igs;
	std::shared_ptr<TextSearchState> tss;
	std::shared_ptr<ResultListState> rls;
};

} //end namespace oscar_gui


#endif
