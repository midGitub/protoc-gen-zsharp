﻿using System;
using System.Collections;
using System.Collections.Generic;
using Google.Protobuf;
using Google.Protobuf.Collections;
using Zynga.Protobuf.Runtime.EventSource;

namespace Zynga.Protobuf.Runtime {
	/// <summary>
	/// Wraps a repeated field to add event sourcing support
	/// </summary>
	public abstract class EventRepeatedField<T> : IList<T>, ICollection<T>, IEnumerable<T>, IEnumerable, IList, ICollection, IDeepCloneable<RepeatedField<T>>, IEquatable<RepeatedField<T>>, IReadOnlyList<T>, IReadOnlyCollection<T> {
		private readonly RepeatedField<T> _internal = new RepeatedField<T>();
		private readonly List<EventData> _root;
		private readonly EventPath _path;
		private readonly EventDataConverter<T> _converter;
		
		public EventRepeatedField(List<EventData> root, EventPath path, EventDataConverter<T> converter) {
			_root = root;
			_path = path;
			_converter = converter;
		}
		
		public RepeatedField<T> Clone() {
			return _internal.Clone();
		}

		public void AddEntriesFrom(CodedInputStream input, FieldCodec<T> codec) {
			_internal.AddEntriesFrom(input, codec);
		}

		public int CalculateSize(FieldCodec<T> codec) {
			return _internal.CalculateSize(codec);
		}

		public void WriteTo(CodedOutputStream output, FieldCodec<T> codec) {
			_internal.WriteTo(output, codec);
		}

		public void Add(T item) {
			_internal.Add(item);
			AddEvent(EventAction.AddList, Count, _converter.GetEventData(item));
		}

		public void Clear() {
			_internal.Clear();
			AddEvent(EventAction.ClearList);
		}

		public bool Contains(T item) {
			return _internal.Contains(item);
		}

		public void CopyTo(T[] array, int arrayIndex) {
			_internal.CopyTo(array, arrayIndex);
		}

		public bool Remove(T item) {
			var result = _internal.Remove(item);
			if (result) {
				AddEvent(EventAction.RemoveList, _converter.GetEventData(item));
			}

			return result;
		}

		public int Count {
			get { return _internal.Count; }
		}

		public bool IsReadOnly {
			get { return _internal.IsReadOnly; }
		}

		public IEnumerator<T> GetEnumerator() {
			return _internal.GetEnumerator();
		}

		public override bool Equals(object obj) {
			return _internal.Equals(obj);
		}

		IEnumerator IEnumerable.GetEnumerator() {
			return (IEnumerator) _internal.GetEnumerator();
		}

		public override int GetHashCode() {
			return _internal.GetHashCode();
		}

		public bool Equals(RepeatedField<T> other) {
			return _internal.Equals(other);
		}

		public int IndexOf(T item) {
			return _internal.IndexOf(item);
		}

		public void Insert(int index, T item) {
			_internal.Insert(index, item);
			AddEvent(EventAction.InsertList, index, _converter.GetEventData(item));
		}

		public void RemoveAt(int index) {
			_internal.RemoveAt(index);
			AddEvent(EventAction.RemoveAtList, index);
		}

		public override string ToString() {
			return _internal.ToString();
		}

		public T this[int index] {
			get { return _internal[index]; }
			set {
				_internal[index] = value;
				AddEvent(EventAction.ReplaceList, index, _converter.GetEventData(value));
			}
		}

		bool IList.IsFixedSize {
			get { return false; }
		}

		void ICollection.CopyTo(Array array, int index) {
			_internal.CopyTo((T[]) array, index);
		}

		bool ICollection.IsSynchronized {
			get { return false; }
		}

		object ICollection.SyncRoot {
			get { return (object) this; }
		}

		object IList.this[int index] {
			get { return (object) this[index]; }
			set { this[index] = (T) value; }
		}

		int IList.Add(object value) {
			Add((T) value);
			return Count - 1;
		}

		bool IList.Contains(object value) {
			if (value is T)
				return Contains((T) value);
			return false;
		}

		int IList.IndexOf(object value) {
			if (!(value is T))
				return -1;
			return IndexOf((T) value);
		}

		void IList.Insert(int index, object value) {
			Insert(index, (T) value);
		}

		void IList.Remove(object value) {
			if (!(value is T))
				return;
			Remove((T) value);
		}
		
		private void AddEvent(EventAction action) {
			var newEvent = new EventData {
				Action = action,
			};
			newEvent.Path.AddRange(_path._path);
			_root.Add(newEvent);
		}
		
		private void AddEvent(EventAction action, int field) {
			var newEvent = new EventData {
				Action = action,
				Field = field
			};
			newEvent.Path.AddRange(_path._path);
			_root.Add(newEvent);
		}
		
		private void AddEvent(EventAction action, EventContent data) {
			var newEvent = new EventData {
				Action = action,
				Data = data
			};
			newEvent.Path.AddRange(_path._path);
			_root.Add(newEvent);
		}
		
		private void AddEvent(EventAction action, int field, EventContent data) {
			var newEvent = new EventData {
				Action = action,
				Field = field,
				Data = data
			};
			newEvent.Path.AddRange(_path._path);
			_root.Add(newEvent);
		}

		public bool ApplyEvent(EventData e) {
			switch (e.Action) {
				case EventAction.AddList:
					_internal.Add(_converter.GetItem(e.Data));
					return true;
				case EventAction.RemoveList:
					_internal.Remove(_converter.GetItem(e.Data));
					return true;
				case EventAction.RemoveAtList:
					_internal.RemoveAt(e.Field);
					return true;
				case EventAction.ReplaceList:
					_internal[e.Field] = _converter.GetItem(e.Data);
					return true;
				case EventAction.InsertList:
					_internal.Insert(e.Field, _converter.GetItem(e.Data));
					return true;
				case EventAction.ClearList:
					_internal.Clear();
					return true;
				default:
					return false;
			}
		}
	}
}