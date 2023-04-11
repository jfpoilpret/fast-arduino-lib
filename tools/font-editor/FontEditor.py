#!/usr/bin/python
# encoding: utf-8

#
#   Copyright 2016-2022 Jean-Francois Poilpret
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
 
# This python mini-application allows creation of display fonts and creates CPP files
# (header & source) from created fonts.

#TODO function to convert glyphs to XBM? Then use tkinter.BitmapImage
#	-> https://stackoverflow.com/a/12287117 to use BitmapImage and set individual pixels!
#TODO add panel to display all characters from font: expected vs actual?
#TODO use characters panel to select one character (remove combo)
#TODO Generate horizontal fonts too

#TODO add menu/buttons for actions (replace CLI args)
# - new font with dialog (first & last char, size, name)
# - open font (for update) dialog to find in directory
# - export font: dialog options (vertical), directory save

from argparse import *
import io
from os import system
import pickle
import re
import sys

from tkinter import *
from tkinter import ttk

from font_export import generate_fastarduino_header, generate_fastarduino_source, generate_regular_code

class FontPersistence:
	def __init__(self, name:str, width: int, height: int, first: int, last: int):
		self.name = name
		self.width = width
		self.height = height
		self.first = first
		self.last = last
		self.glyphs = {chr(c): [] for c in range(first, last + 1)}

class Pixel(ttk.Label):
	def __init__(self, master: Misc, value: bool, **kwargs):
		super().__init__(master, **kwargs)
		self.set_value(value)
	
	def get_value(self):
		return self.value
	
	def set_value(self, value: bool):
		self.value = value
		if self.value:
			self.configure(image = Pixel.BLACK)
		else:
			self.configure(image = Pixel.WHITE)

#TODO Most efficient way to draw thumbnail fo glyphs?
class CharacterThumbnail(ttk.Frame):
	PIX_SIZE = 2

	def __init__(self, master: Misc, width: int, height: int) -> None:
		super().__init__(master, padding=1, border=1)
		self.glyph_height = height
		self.glyph_width = width
		self.letter_label = ttk.Label(self)
		self.letter_label.grid(row=0, column=0)
		self.glyph_label = ttk.Label(self)
		self.glyph_label.grid(row=1, column=0)
		self.glyph_image = PhotoImage(master=self, 
			width=width*CharacterThumbnail.PIX_SIZE, height=height*CharacterThumbnail.PIX_SIZE)
		self.glyph_label.config(image=self.glyph_image)
	
	def set_character(self, char):
		self.letter_label.config(text=f"{ord(char):02x} ({char})")
	
	def clear_glyph(self):
		data: list[list[str]] = []
		for r in range(self.glyph_height * CharacterThumbnail.PIX_SIZE):
			row = []
			for c in range(self.glyph_width * CharacterThumbnail.PIX_SIZE):
				row.append("white")
			data.append(row)
		self.glyph_image.put(data=data)
	
	def set_glyph(self, glyph: list[list[bool]]):
		data: list[list[str]] = []
		for r in range(self.glyph_height * CharacterThumbnail.PIX_SIZE):
			row = []
			for c in range(self.glyph_width * CharacterThumbnail.PIX_SIZE):
				row.append("white")
			data.append(row)
			
		for r, row in enumerate(glyph):
			for c, col in enumerate(row):
				if col:
					c1 = c * CharacterThumbnail.PIX_SIZE
					for x in range(c1, c1 + CharacterThumbnail.PIX_SIZE):
						r1 = r * CharacterThumbnail.PIX_SIZE
						for y in range(r1, r1 + CharacterThumbnail.PIX_SIZE):
							data[y][x] = "black"
		self.glyph_image.put(data=data)

class FontEditor(ttk.Frame):
	def __init__(self, master: Misc, font_state: FontPersistence):
		super().__init__(master, padding = (4, 4, 4, 4))
		self.font_state = font_state
		self.previous_char = None
		self.current_char = StringVar()
		self.grid(column = 0, row = 0, sticky = (N))
		master.columnconfigure(0, weight = 1)
		master.rowconfigure(0, weight = 1)
		# create widgets
		ttk.Label(self, text = 'Character:').grid(row = 1, column = 1, padx = 3, pady = 3)
		all_characters = [chr(c) for c in range(font_state.first, font_state.last + 1)]
		char_selector = ttk.Combobox(self, state = 'readonly', values = all_characters, textvariable = self.current_char)
		char_selector.grid(row = 1, column = 2, padx = 3, pady = 3)
		char_selector.bind('<<ComboboxSelected>>', self.on_char_select)
		ttk.Button(self, text = 'Save', command = self.on_save).grid(row = 1, column = 3, padx = 3, pady = 3)
		
		#TODO thumbnail somehow
		# At first, just create one thumbnail (for testing)
		self.thumbnail = CharacterThumbnail(master=self, width=font_state.width, height=font_state.height)
		self.thumbnail.grid(row=2, column=3)

		#TODO need a grid to show all BPM: determine rows and cols first
		#TODO need a widget combining font char expected (standard font) Vs actual (updated font)

		# Panel for pixmap editing
		Pixel.WHITE = PhotoImage(file = 'white.png')
		Pixel.BLACK = PhotoImage(file = 'black.png')
		size = Pixel.WHITE.width() + 1
		self.pixmap_editor = ttk.Frame(self)
		self.pixmap_editor.grid(row = 2, column = 1, columnspan = 2)
		self.pixmap_editor.configure(width = size * self.font_state.width, height = size * self.font_state.height)
		self.pixmap_editor.bind('<Button-1>', self.on_pixel_click, )
		self.pixmap_editor.bind('<B1-Motion>', self.on_pixel_move)
		self.pixels = []
		# Initialize all image labels
		for y in range(self.font_state.height):
			row = []
			for x in range(self.font_state.width):
				pixel = Pixel(self.pixmap_editor, value = False, borderwidth = 1)
				pixel.place(x = x * size, y = y * size)
				# Trick to ensure events pass through from Pixel instacnes to parent Frame
				bindtags = list(pixel.bindtags())
				bindtags.insert(1, self.pixmap_editor)
				pixel.bindtags(tuple(bindtags))
				pixel.bind('<Button-1>', lambda e: None)
				pixel.bind('<B1-Motion>', lambda e: None)
				row.append(pixel)
			self.pixels.append(row)

	def get_glyph_from_pixels(self):
		glyph = []
		for pixels_row in self.pixels:
			row = []
			for pixel in pixels_row:
				row.append(pixel.get_value())
			glyph.append(row)
		return glyph

	def get_xbm_from_glyph(self, glyph):
		#TODO
		pass
	
	def update_pixels_from_glyph(self, glyph):
		for y in range(self.font_state.height):
			glyph_row = glyph[y]
			pixels_row = self.pixels[y]
			for x in range(self.font_state.width):
				glyph_pixel: bool = glyph_row[x]
				pixel: Pixel = pixels_row[x]
				pixel.set_value(glyph_pixel)

	def clear_pixels(self):
		for y in range(self.font_state.height):
			pixels_row = self.pixels[y]
			for x in range(self.font_state.width):
				pixel: Pixel = pixels_row[x]
				pixel.set_value(False)
	
	def find_pixel(self, x: int, y: int) -> Pixel:
		size = Pixel.WHITE.width() + 1
		row = int(y / size)
		col = int(x / size)
		# row or col may be out of range when dragging ouside the pane
		if 0 <= row < self.font_state.height and 0 <= col < self.font_state.width:
			return self.pixels[row][col]
		else:
			return None

	def on_char_select(self, event: Event):
		# Update font_state with previous character
		if self.previous_char:
			glyph = self.get_glyph_from_pixels()
			self.font_state.glyphs[self.previous_char] = glyph

		# Load pixmap of new current character from font_state
		self.previous_char = self.current_char.get()
		self.thumbnail.set_character(self.previous_char)
		glyph = self.font_state.glyphs[self.previous_char]
		if glyph:
			self.update_pixels_from_glyph(glyph)
			self.thumbnail.set_glyph(glyph=glyph)
		else:
			# no glyph yet, set all white pixels
			self.clear_pixels()
			self.thumbnail.clear_glyph()

	def on_pixel_click(self, event: Event):
		# print(f'on_pixel_click ({event.x},{event.y})')
		pixel: Pixel = event.widget
		pixel = self.find_pixel(event.x + pixel.winfo_x(), event.y + pixel.winfo_y())
		# Invert image W->B, B->W
		self.default_pixel_value = not pixel.get_value()
		pixel.set_value(self.default_pixel_value)

	def on_pixel_move(self, event: Event):
		# print(f'on_pixel_move ({event.x},{event.y})')
		pixel: Pixel = event.widget
		pixel = self.find_pixel(event.x + pixel.winfo_x(), event.y + pixel.winfo_y())
		if pixel:
			pixel.set_value(self.default_pixel_value)

	def on_save(self):
		# Update glyph of current character
		if self.previous_char:
			glyph = self.get_glyph_from_pixels()
			self.font_state.glyphs[self.previous_char] = glyph
		# Save font state to storage
		with open(self.font_state.name + '.font', 'wb') as output:
			pickle.dump(self.font_state, file = output)

def create(name: str, font_width: int, font_height: int, first_char: str, last_char: str):
	# Create FontPersistence
	font_state = FontPersistence(name, font_width, font_height, ord(first_char), ord(last_char))
	# Create Window
	root = Tk()
	root.wm_title(f'Editor for font `{font_state.name}` ({font_state.width}x{font_state.height})')
	app = FontEditor(root, font_state)
	root.mainloop()

def update(name: str):
	# Read FontPersistence from storage
	with open(name + '.font', 'rb') as input:
		font_state: FontPersistence = pickle.load(input)
	# Create Window
	root = Tk()
	root.wm_title(f'Editor for font `{font_state.name}` ({font_state.width}x{font_state.height})')
	app = FontEditor(root, font_state)
	root.mainloop()

def export(name: str, vertical: bool, fastarduino: bool, filename: str):
	# Read FontPersistence from storage
	with open(name + '.font', 'rb') as input:
		font_state: FontPersistence = pickle.load(input)
	# Check all characters have been defined in font state
	for c, glyph in font_state.glyphs.items():
		if not glyph:
			print(f"Glyph for character '{c}' is undefined!")
			sys.exit(1)
	if fastarduino:
		# Generate header file content
		header = generate_fastarduino_header(filename, font_state.name, 
			font_state.width, font_state.height,
			font_state.first, font_state.last, vertical)
		with open(filename + '.h', 'wt') as output:
			output.write(header)
		# Generate source file content
		source = generate_fastarduino_source(filename, font_state.name,
			font_state.width, font_state.height, font_state.first, font_state.last,
			vertical, font_state.glyphs)
		with open(filename + '.cpp', 'wt') as output:
			output.write(source)
	else:
		# Generate regular source code (for specific program use)
		source = generate_regular_code(filename, font_state.name,
		font_state.width, font_state.height, font_state.first, font_state.last,
		vertical, font_state.glyphs)
		with open(filename + '.h', 'wt') as output:
			output.write(source)

class FontSizeAction(Action):
	def __init__(self, **kwargs):
		self.matcher = re.compile('([1-9][0-9]*)x([1-9][0-9]*)')
		super().__init__(**kwargs)
	
	def __call__(self, parser: ArgumentParser, namespace: Namespace, values: str, option_string = None):
		match = self.matcher.match(values)
		if not match:
			parser.error('--font-size must have format WxH with both W and H integral values')
		setattr(namespace, 'font_width', int(match.group(1)))
		setattr(namespace, 'font_height', int(match.group(2)))

if __name__ == '__main__':
	# --name NAME --font-size WxH --vertical --first A --last Z
	parser = ArgumentParser(description = 'Font editor for FastArduino Font subclasses')
	group = parser.add_subparsers(dest = 'action', required = True)

	group_create = group.add_parser('create', help = 'create help')
	group_create.add_argument('--name', type = str, required = True, help = 'Font name (Font subclass name)')
	group_create.add_argument('--font-size', type = str, action = FontSizeAction, required = True, help = 'Font size in pixels, represented as WxH')
	group_create.add_argument('--first-char', type = lambda x: x if x.isalpha() and len(x) == 1 else False, required = True, help = 'Font first supported character')
	group_create.add_argument('--last-char', type = lambda x: x if x.isalpha() and len(x) == 1 else False, required = True, help = 'Font last supported character')
	
	group_work = group.add_parser('update', help = 'update help')
	group_work.add_argument('--name', type = str, required = True, help = 'Font name (Font subclass name)')
	
	group_export = group.add_parser('export', help = 'export help')
	group_export.add_argument('--name', type = str, required = True, help = 'Font name (Font subclass name)')
	group_export.add_argument('--vertical', action = 'store_true', help = 'Produce vertical font')
	group_export.add_argument('--fastarduino', action = 'store_true', help = 'Generated files are for inclusion to FastArduino library')
	group_export.add_argument('--filename', type = str, required = True, help = 'Root name fo C++ header and source files to generate for the font')
	
	args = parser.parse_args()
	if args.action == 'create':
		create(args.name, args.font_width, args.font_height, args.first_char, args.last_char)
	elif args.action == 'update':
		update(args.name)
	elif args.action == 'export':
		export(args.name, args.vertical, args.fastarduino, args.filename)
	else:
		print("Impossible arguments situation! You must select create, update or export commands!")
