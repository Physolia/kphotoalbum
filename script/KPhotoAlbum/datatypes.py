class Category(object):
	"""
	Stores category information.
	"""
	def __init__(self, name, icon,
		     visible, viewtype, viewsize,
		     items=None):
		self.name = name
		self.icon = icon
		self.visible = visible
		self.viewtype = viewtype
		self.viewsize = viewsize
		self.items = items
		if self.items is None:
			self.items = {}

	def addItem(self, name, id):
		assert not self.items.has_key(id)
		self.items[id] = name

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.name) + ', ' +
		     repr(self.icon) + ', ' +
		     repr(self.visible) + ', ' +
		     repr(self.viewtype) + ', ' +
		     repr(self.viewsize))
		if len(self.items) > 0:
			s += ', ' + repr(self.items)
		s += ')'
		return s


class MediaItem(object):
	"""
	Stores media item information.
	"""
	def __init__(self, label, description,
		     filename, md5sum,
		     startDate, endDate,
		     width, height, angle,
		     tags=None, drawings=None):
		self.label = label
		self.description = description
		self.filename = filename
		self.md5sum = md5sum
		self.startDate = startDate
		self.endDate = endDate
		self.width = width
		self.height = height
		self.angle = angle
		self.tags = tags
		self.drawings = drawings
		if self.tags is None:
			self.tags = set()
		if self.drawings is None:
			self.drawings = []

	def addTag(self, tag):
		self.tags.add(tag)

	def addDrawing(self, drawing):
		self.drawings += [drawing]

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.label) + ', ' +
		     repr(self.description) + ', ' +
		     repr(self.filename) + ', ' +
		     repr(self.md5sum) + ', ' +
		     repr(self.startDate) + ', ' +
		     repr(self.endDate) + ', ' +
		     repr(self.width) + ', ' +
		     repr(self.height) + ', ' +
		     repr(self.angle))
		if len(self.tags) > 0:
			s += ', tags=' + repr(self.tags)
		if len(self.drawings) > 0:
			s += ', drawings=' + repr(self.drawings)
		s += ')'
		return s


class Tag(object):
	def __init__(self, category, name):
		self.category = category
		self.name = name


	def __getitem__(self, i):
		if i == 0:
			return self.category
		elif i == 1:
			return self.name
		else:
			raise IndexError('index should be 0 or 1')

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.category) + ', ' +
			repr(self.name) + ')')


class Drawing(object):
	def __init__(self, shape, point0, point1):
		assert shape in ['circle', 'line', 'rectangle']
		self.shape = shape
		self.point0 = point0
		self.point1 = point1

	def __eq__(self, other):
		return (self.shape == other.shape and
			self.point0 == other.point0 and
			self.point1 == other.point1)

	def __hash__(self):
		return (((ord(self.shape[0]) & 7) << 28) |
			((hash(self.point0) & ((1 << 14) - 1)) << 14) |
			(hash(self.point1) & ((1 << 14) - 1)))

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.shape) + ', ' +
			repr(self.point0) + ', ' +
			repr(self.point1) + ')')


class MemberGroup(Tag):
	def __init__(self, category, name, members=None):
		super(MemberGroup, self).__init__(category, name)
		self.members = members
		if self.members is None:
			self.members = []

	def addMember(self, member):
		self.members += [member]

	def __repr__(self):
		s = (self.__class__.__name__ + '(' +
		     repr(self.category) + ', ' +
		     repr(self.name))
		if len(self.members) > 0:
			s += ', ' + repr(self.members)
		return s + ')'

class BlockItem(object):
	def __init__(self, filename):
		self.filename = filename

	def __repr__(self):
		return (self.__class__.__name__ + '(' +
			repr(self.filename) + ')')
