#!/usr/bin/python
# encoding: utf-8

#
#   Copyright 2016-2023 Jean-Francois Poilpret
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

#TODO implement menu items
# 1h	- revert current glyph change (before save)
# 2h	- extend/reduce font range

#TODO 2h	UI fine-tune (menu accelerators...)
#TODO 4h	Generate horizontal fonts too
#TODO 2h	Refactoring to make code better and easier to read and maintain (use Tk vars?)
#TODO 30'	Refactor to put exporting functions here too (only one source code file)

from dataclasses import dataclass
import pickle
import re
import sys

from tkinter import *
from tkinter import filedialog, messagebox
from tkinter import ttk

from font_export import generate_fastarduino_header, generate_fastarduino_source, generate_regular_code

# Utility to trick events handling so that events pass through from a widget to a parent widget
def pass_events_to_parent(widget: Widget, parent: Widget, events: list[str]):
	bindtags = list(widget.bindtags())
	bindtags.insert(1, parent)
	widget.bindtags(tuple(bindtags))
	for event in events:
		widget.bind(event, lambda e: None)

# Class embedding font state for persistence
class FontPersistence:
	def __init__(self, name:str, width: int, height: int, first: int, last: int):
		self.name = name
		self.width = width
		self.height = height
		self.first = first
		self.last = last
		self.glyphs = {chr(c): [] for c in range(first, last + 1)}

# Dialog displayed at creation fo a new font
class NewFontDialog(Toplevel):
	def __init__(self, master: Misc):
		super().__init__(master=master)
		self.title("Font Settings")
		self.result: FontPersistence | None = None
		# These variables will get user input
		self.width = IntVar(value=8)
		self.height = IntVar(value=8)
		self.first = StringVar()
		self.last = StringVar()
		# Add UI: font width and height, 1st char, last char
		ttk.Label(master=self, text="Width:").grid(row=1, column=1, padx=2, pady=2, sticky="W")
		focus_entry = ttk.Spinbox(master=self, from_=4, to=64, increment=1, textvariable=self.width)
		focus_entry.grid(row=1, column=2, padx=2, pady=2)
		focus_entry.focus()
		ttk.Label(master=self, text="Height:").grid(row=2, column=1, padx=2, pady=2, sticky="W")
		ttk.Spinbox(master=self, from_=4, to=64, increment=1, textvariable=self.height).grid(
			row=2, column=2, padx=2, pady=2)
		ttk.Label(master=self, text="First letter:").grid(row=3, column=1, padx=2, pady=2, sticky="W")
		ttk.Spinbox(master=self, values=[chr(c) for c in range(32, 127)], textvariable=self.first).grid(
			row=3, column=2, padx=2, pady=2)
		ttk.Label(master=self, text="Last letter:").grid(row=4, column=1, padx=2, pady=2, sticky="W")
		ttk.Spinbox(master=self, values=[chr(c) for c in range(32, 127)], textvariable=self.last).grid(
			row=4, column=2, padx=2, pady=2)
		buttons = ttk.Frame(master=self)
		ttk.Button(master=buttons, text="Cancel", command=self.on_cancel).grid(row=5, column=1, padx=2, pady=2)
		ttk.Button(master=buttons, text="OK", command=self.on_ok).grid(row=5, column=2, padx=2, pady=2)
		buttons.grid(row=5, column=1, columnspan=2)

		self.protocol(name="WM_DELETE_WINDOW", func=self.on_cancel)
		self.transient(master=master)
		self.wait_visibility()
		self.grab_set()
		self.wait_window()

	def get_font_state(self) -> FontPersistence | None:
		return self.result
	
	def on_ok(self):
		first = ord(self.first.get())
		last = ord(self.last.get())
		if last < first:
			temp = first
			first = last
			last = temp
		self.result = FontPersistence(name=None, width=self.width.get(), height=self.height.get(),
			first=first, last=last)
		self.grab_release()
		self.destroy()

	def on_cancel(self):
		self.grab_release()
		self.destroy()

@dataclass(kw_only=True)
class ExportConfig:
	fastarduino: bool = False
	vertical: bool = False
	directory: str

# Dialog displayed at export of current font
class ExportDialog(Toplevel):
	def __init__(self, master: Misc):
		super().__init__(master=master)
		self.title("Export Settings")
		self.result: ExportConfig | None = None
		# These variables will get user input
		self.fastarduino = BooleanVar()
		self.vertical = BooleanVar()
		self.directory = StringVar()
		# Add UI: font width and height, 1st char, last char
		ttk.Checkbutton(master=self, text="FastArduino Font", variable=self.fastarduino).grid(
			row=1, column=1, columnspan=2, padx=2, pady=2, sticky="W")
		ttk.Checkbutton(master=self, text="Vertical Font", variable=self.vertical).grid(
			row=2, column=1, columnspan=2, padx=2, pady=2, sticky="W")
		ttk.Button(master=self, text="Code Files Directory...", command=self.on_select_dir).grid(
			row=3, column=1, padx=2, pady=2)
		ttk.Label(master=self, textvariable=self.directory).grid(row=3, column=2, padx=2, pady=2)
		buttons = ttk.Frame(master=self)
		ttk.Button(master=buttons, text="Cancel", command=self.on_cancel).grid(row=1, column=1, padx=2, pady=2)
		ttk.Button(master=buttons, text="OK", command=self.on_ok).grid(row=1, column=2, padx=2, pady=2)
		buttons.grid(row=4, column=1, columnspan=2)

		self.protocol(name="WM_DELETE_WINDOW", func=self.on_cancel)
		self.transient(master=master)
		self.wait_visibility()
		self.grab_set()
		self.wait_window()

	def get_export_config(self):
		return self.result
	
	def on_select_dir(self):
		directory = filedialog.askdirectory(title="Select directory to save source code files")
		if directory:
			self.directory.set(directory)
	
	def on_ok(self):
		# Check that directory is selected
		if not self.directory.get():
			messagebox.showwarning(title="Warning", 
				message="Please choose a directory to which source code files will be saved.")
			return
		self.result = ExportConfig(fastarduino=self.fastarduino.get(), vertical=self.vertical.get(),
			directory=self.directory.get())
		self.grab_release()
		self.destroy()
	
	def on_cancel(self):
		self.grab_release()
		self.destroy()

# Pixel widget (just black or white rectangle)
class Pixel(ttk.Label):
	def __init__(self, master: Misc, value: bool, **kwargs):
		super().__init__(master, **kwargs)
		self.set_value(value)
	
	def get_value(self):
		return self.value
	
	def set_value(self, value: bool):
		self.value = value
		if self.value:
			self.configure(image=Pixel.BLACK)
		else:
			self.configure(image=Pixel.WHITE)

# Pane for editing one glyph
class GlyphEditor(ttk.Frame):
	def __init__(self, master: Misc, width: int, height: int) -> None:
		super().__init__(master=master)
		self.width = width
		self.height = height
		Pixel.WHITE = PhotoImage(file='white.png')
		Pixel.BLACK = PhotoImage(file='black.png')
		size = Pixel.WHITE.width() + 1
		self.configure(width=size * width, height=size * height)
		self.bind('<Button-1>', self.on_pixel_click)
		self.bind('<B1-Motion>', self.on_pixel_move)
		self.pixels: list[list[Pixel]] = []
		# Initialize all image labels
		for y in range(height):
			row = []
			for x in range(width):
				pixel = Pixel(self, value=False, borderwidth=1)
				pixel.place(x=x * size, y=y * size)
				# Ensure events pass through from Pixel instances to parent Frame
				pass_events_to_parent(pixel, self, ["<Button-1>", "<B1-Motion>"])
				row.append(pixel)
			self.pixels.append(row)

	def get_glyph_from_pixels(self) -> list[list[bool]]:
		glyph = []
		for pixels_row in self.pixels:
			row = []
			for pixel in pixels_row:
				row.append(pixel.get_value())
			glyph.append(row)
		return glyph

	def update_pixels_from_glyph(self, glyph: list[list[bool]]):
		for y in range(self.height):
			glyph_row = glyph[y]
			pixels_row = self.pixels[y]
			for x in range(self.width):
				glyph_pixel: bool = glyph_row[x]
				pixel: Pixel = pixels_row[x]
				pixel.set_value(glyph_pixel)

	def clear_pixels(self):
		for y in range(self.height):
			pixels_row = self.pixels[y]
			for x in range(self.width):
				pixel: Pixel = pixels_row[x]
				pixel.set_value(False)
	
	def find_pixel(self, x: int, y: int) -> Pixel:
		size = Pixel.WHITE.width() + 1
		row = int(y / size)
		col = int(x / size)
		# row or col may be out of range when dragging ouside the pane
		if 0 <= row < self.height and 0 <= col < self.width:
			return self.pixels[row][col]
		else:
			return None

	def on_pixel_click(self, event: Event):
		pixel: Pixel = event.widget
		pixel = self.find_pixel(event.x + pixel.winfo_x(), event.y + pixel.winfo_y())
		# Invert image W->B, B->W
		self.default_pixel_value = not pixel.get_value()
		pixel.set_value(self.default_pixel_value)

	def on_pixel_move(self, event: Event):
		pixel: Pixel = event.widget
		pixel = self.find_pixel(event.x + pixel.winfo_x(), event.y + pixel.winfo_y())
		if pixel:
			pixel.set_value(self.default_pixel_value)

# Widget reprenting the thumbnail for a given font character
class CharacterThumbnail(Frame):
	PIX_SIZE = 2

	def __init__(self, master: Misc, width: int, height: int) -> None:
		super().__init__(master, padx=1, pady=1, border=1, background="white", borderwidth=2, relief='solid')
		self.char: str = None
		self.glyph_height = height
		self.glyph_width = width
		self.letter_label = Label(self, background="white", font="TkFixedFont")
		self.letter_label.grid(row=0, column=0)
		self.glyph_label = Label(self, background="white")
		self.glyph_label.grid(row=1, column=0)
		self.glyph_image = PhotoImage(master=self, 
			width=width*CharacterThumbnail.PIX_SIZE, height=height*CharacterThumbnail.PIX_SIZE)
		self.glyph_label.config(image=self.glyph_image)
		# Ensure events pass through from labels to parent Frame (self)
		pass_events_to_parent(self.letter_label, self, ["<Button-1>"])
		pass_events_to_parent(self.glyph_label, self, ["<Button-1>"])

	def set_highlight(self, highlight: bool):
		if highlight:
			self.configure(background="yellow")
			self.letter_label.configure(background="yellow")
			self.glyph_label.configure(background="yellow")
		else:
			self.configure(background="white")
			self.letter_label.configure(background="white")
			self.glyph_label.configure(background="white")
	
	def get_character(self) -> str:
		return self.char
	
	def set_character(self, char: str):
		self.char = char
		self.letter_label.config(text=f"{ord(char):02x}-{char}")
	
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

# Panel containing all thumbnails for the current font
class ThumbnailPanel(Frame):
	MAX_GRIDX = 8

	def __init__(self, master: Misc, font_state: FontPersistence) -> None:
		super().__init__(master, background="white")
		# create CharacterThumbnail for each character in the font
		self.thumbnails: dict[str, CharacterThumbnail] = {}
		self.selected_thumbnail: CharacterThumbnail = None
		gridx = 1
		gridy = 1
		for c in range(font_state.first, font_state.last + 1):
			thumb = CharacterThumbnail(master=self, width=font_state.width, height=font_state.height)
			thumb.bind(sequence="<Button-1>", func=master.on_thumbnail_click)
			thumb.set_character(char=chr(c))
			thumb.set_glyph(glyph=font_state.glyphs[chr(c)])
			self.thumbnails[chr(c)] = thumb
			# Add thumbnail to the grid
			thumb.grid(row=gridy, column=gridx)
			gridx += 1
			if gridx > ThumbnailPanel.MAX_GRIDX:
				gridx = 1
				gridy += 1
	
	def select_character(self, c: str) -> None:
		if self.selected_thumbnail:
			self.selected_thumbnail.set_highlight(highlight=False)
		self.selected_thumbnail = self.thumbnails[c]
		self.selected_thumbnail.set_highlight(highlight=True)
	
	def update_character(self, c: str, glyph: list[list[bool]]) -> None:
		self.thumbnails[c].set_glyph(glyph=glyph)
	
	def update_all(self, font_state: FontPersistence) -> None:
		for c, glyph in font_state.glyphs.items():
			self.thumbnails[c].set_glyph(glyph=glyph)

class FontEditor(ttk.Frame):
	def __init__(self, master: Tk):
		super().__init__(master, padding=(4, 4, 4, 4))
		master.option_add("*tearOff", False)
		master.minsize(width=300, height=200)

		self.filename: str = None
		self.font_state: FontPersistence = None
		self.previous_char: str = None
		self.thumbnails: ThumbnailPanel = None
		self.glyph_editor: GlyphEditor = None
		self.is_dirty: bool = False
		self.clipboard: list[list[bool]] = None

		# Add menu bar here
		#TODO Add accelerators and underlines
		menubar = Menu(master)
		master['menu'] = menubar
		menu_file = Menu(menubar)
		menu_edit = Menu(menubar)
		menubar.add_cascade(menu=menu_file, label="File")
		menubar.add_cascade(menu=menu_edit, label="Edit")

		menu_file.add_command(label="New Font...", command=self.on_new)
		menu_file.add_command(label="Open Font...", command=self.on_open)
		menu_file.add_separator()
		menu_file.add_command(label="Save", command=self.on_save)
		menu_file.add_command(label="Export...", command=self.on_export)
		menu_file.add_separator()
		menu_file.add_command(label="Close Font", command=self.on_close)
		menu_file.add_separator()
		menu_file.add_command(label="Quit", command=self.on_quit)

		menu_edit.add_command(label="Undo", command=self.on_undo)
		menu_edit.add_separator()
		menu_edit.add_command(label="Copy", command=self.on_copy)
		menu_edit.add_command(label="Paste", command=self.on_paste)
		menu_edit.add_separator()
		menu_edit.add_command(label="Change Font Range...", command=self.on_change_font_range)

		self.grid(column=0, row=0, sticky=(N))
		master.columnconfigure(0, weight=1)
		master.rowconfigure(0, weight=1)

		self.update_title()

	def update_title(self):
		state = self.font_state
		if state:
			title = f"Editor for font `{state.name}` ({state.width}x{state.height})"
			if self.is_dirty:
				title += " *"
			self.master.title(title)
		else:
			self.master.title("Editor for FastArduino fonts")
	
	def set_font(self, font_state: FontPersistence):
		self.font_state = font_state
		self.previous_char: str = None

		# Remove thumbnails and glyph editor panes if they already exist
		if self.thumbnails:
			self.thumbnails.grid_remove()
		if self.glyph_editor:
			self.glyph_editor.grid_remove()

		# Add new thumbnail pane
		self.thumbnails = ThumbnailPanel(master=self, font_state=font_state)
		self.thumbnails.grid(row=1, column=1, padx=3, pady=3)

		# Add new panel for glyph editing
		self.glyph_editor = GlyphEditor(
			master=self, width=self.font_state.width, height=self.font_state.height)
		self.glyph_editor.grid(row=1, column=2, padx=3, pady=3)

		# select 1st thumbnail
		self.select_first()

		self.update_title()

	def select_first(self):
		# select 1st thumbnail
		self.click_thumbnail(self.thumbnails.thumbnails[chr(self.font_state.first)])

	def update_is_dirty(self):
		glyph = self.glyph_editor.get_glyph_from_pixels()
		if glyph != self.font_state.glyphs[self.previous_char]:
			self.is_dirty = True
			self.thumbnails.update_character(self.previous_char, glyph)
			self.font_state.glyphs[self.previous_char] = glyph
			self.update_title()
	
	def check_dirty(self) -> bool:
		if not self.is_dirty:
			return True
		result = messagebox.askyesnocancel(title="", 
			message=f"Font `{self.font_state.name}` has changed.\nDo you want to save it before proceeding?")
		if result == True:
			# Save font
			self.on_save()
			# Continue normally with calling action
			return True
		if result == False:
			# Do not save font & continue normally with calling action
			return True
		# result == None: user cancelled action
		return False
	
	def click_thumbnail(self, thumbnail: CharacterThumbnail) -> None:
		# Update font_state with previous character
		if self.previous_char:
			self.update_is_dirty()
		# Highlight clicked thumbnail
		self.previous_char = thumbnail.get_character()
		self.thumbnails.select_character(self.previous_char)
		# Load pixmap of new current character from font_state
		glyph = self.font_state.glyphs[self.previous_char]
		if glyph:
			self.glyph_editor.update_pixels_from_glyph(glyph)
		else:
			# no glyph yet, set all white pixels
			self.glyph_editor.clear_pixels()

	def on_thumbnail_click(self, event: Event) -> None:
		# Find clicked thumbnail
		thumbnail: CharacterThumbnail = None
		if isinstance(event.widget, CharacterThumbnail):
			thumbnail = event.widget
		else:
			target: Widget = event.widget
			thumbnail = target.master
		self.click_thumbnail(thumbnail=thumbnail)

	def on_new(self):
		# Check if save needed
		if not self.check_dirty(): return
		# Open dialog to select font size and font range
		dialog = NewFontDialog(master=self.master)
		# Get new font info (or None if dialog cancelled)
		font_state = dialog.get_font_state()
		if font_state:
			self.filename = None
			self.set_font(font_state)
	
	def on_open(self):
		# Check if save needed
		if not self.check_dirty(): return
		filename = filedialog.askopenfilename(
			title="Select Font File to Open" ,filetypes=[("Font files", "*.font")])
		if filename:
			self.filename = filename
			# Read FontPersistence from storage and update UI
			with open(filename, 'rb') as input:
				font_state: FontPersistence = pickle.load(input)
				self.set_font(font_state)
	
	def on_close(self):
		# Check if save needed
		if not self.check_dirty(): return
		self.filename = None
		pass
	
	def on_save(self):
		# Check if this is a new font (need to use save dialog)
		if not self.filename:
			# Open save file dialog
			filename = filedialog.asksaveasfilename(
				title="Save Font as", filetypes=[("Font files", "*.font")])
			# Update font_state name according to selected filename
			if not filename:
				#FIXME what to do here?
				return
			matcher = re.search(r"([^/\\]*)\.font$", filename)
			if not matcher:
				#FIXME what to do here?
				return
			self.filename = filename
			self.font_state.name = matcher.group(1)
		# Update glyph of current character
		if self.previous_char:
			glyph = self.glyph_editor.get_glyph_from_pixels()
			self.font_state.glyphs[self.previous_char] = glyph
		self.thumbnails.update_all(font_state=self.font_state)
		# Save font state to storage
		with open(self.filename, 'wb') as output:
			pickle.dump(self.font_state, file = output)

	def on_export(self):
		# Check all characters have been defined in font state
		for c, glyph in self.font_state.glyphs.items():
			if not glyph:
				#TODO show message instead of exiting!!!
				print(f"Glyph for character '{c}' is undefined!")
				sys.exit(1)
		dialog = ExportDialog(master=self.master)
		export_config = dialog.get_export_config()
		if export_config:
			print(f"export_config={export_config}")
			# Perform export
			directory = export_config.directory
			filename = f"{directory}/{self.font_state.name}"
			if export_config.fastarduino:
				# Generate header file content
				header = generate_fastarduino_header(self.font_state.name, self.font_state.name, 
					self.font_state.width, self.font_state.height,
					self.font_state.first, self.font_state.last, export_config.vertical)
				with open(filename + '.h', 'wt') as output:
					output.write(header)
				# Generate source file content
				source = generate_fastarduino_source(self.font_state.name, self.font_state.name,
					self.font_state.width, self.font_state.height, self.font_state.first, self.font_state.last,
					export_config.vertical, self.font_state.glyphs)
				with open(filename + '.cpp', 'wt') as output:
					output.write(source)
			else:
				# Generate regular source code (for specific program use)
				source = generate_regular_code(filename, self.font_state.name,
					self.font_state.width, self.font_state.height, self.font_state.first, self.font_state.last,
					export_config.vertical, self.font_state.glyphs)
				with open(filename + '.h', 'wt') as output:
					output.write(source)
	
	def on_quit(self):
		# Check if save needed
		if not self.check_dirty(): return
		self.master.destroy()
		pass
	
	def on_copy(self):
		# Get current character glyph and copy it to self.clipboard (deep copy)
		self.clipboard = self.glyph_editor.get_glyph_from_pixels()
		pass
	
	def on_paste(self):
		if self.clipboard:
			# Deep copy clipboard content to current character glyph
			self.glyph_editor.update_pixels_from_glyph(self.clipboard)
			pass
	
	def on_undo(self):
		#TODO fourth better revert than undo (easier), all changes or only current glyph?
		pass

	def on_change_font_range(self):
		#TODO fifth need dialog (looks like NewFontDialog without font size and default values)
		pass

if __name__ == '__main__':
	# Create Window
	root = Tk()
	app = FontEditor(root)
	root.mainloop()
