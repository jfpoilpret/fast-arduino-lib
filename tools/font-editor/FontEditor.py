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

#TODO 1h+	Move edited glyph (up/down/left/right) -> difficulty will be UNDO
#TODO 4h	Generate horizontal fonts too

from dataclasses import dataclass
import pickle
import re

from tkinter import *
from tkinter import filedialog, messagebox
from tkinter import ttk

MIN_CHAR_CODE = 32
MAX_CHAR_CODE = 127
CHAR_VALUES = [chr(c) for c in range(MIN_CHAR_CODE, MAX_CHAR_CODE)]

MIN_SIZE = 4
DEFAULT_SIZE = 8
MAX_SIZE = 64

# Utility to trick events handling so that events pass through from a widget to a parent widget
def pass_events_to_parent(widget: Widget, parent: Widget, events: list[str]):
	bindtags = list(widget.bindtags())
	bindtags.insert(1, parent)
	widget.bindtags(tuple(bindtags))
	for event in events:
		widget.bind(event, lambda e: None)

# Class embedding font state for persistence
@dataclass(kw_only=True)
class FontState:
	name: str
	width: int
	height: int
	first: int
	last: int
	glyphs: dict[int, list[list[bool]]] = None

	def __post_init__(self):
		self.glyphs: dict[int, list[list[bool]]] = {
			c: self.new_empty_glyph() for c in range(self.first, self.last + 1)}
	
	def new_empty_row(self) ->list[bool]:
		return [False for i in range(self.width)]

	def new_empty_glyph(self) -> list[list[bool]]:
		return [self.new_empty_row() for i in range(self.height)]

	def update_range(self, first: int, last: int):
		if first < self.first:
			self.glyphs.update({c: self.new_empty_glyph() for c in range(first, self.first)})
		elif first > self.first:
			for c in range(self.first, first):
				del self.glyphs[c]
		if last > self.last:
			self.glyphs.update({c: self.new_empty_glyph() for c in range(self.last + 1, last + 1)})
		elif last < self.last:
			for c in range(last + 1, self.last + 1):
				del self.glyphs[c]
		self.first = first
		self.last = last
	
	def update_size(self, width: int, height: int):
		delta_height = height - self.height
		delta_width = width - self.width
		self.width = width
		self.height = height
		extra_width = [False for i in range(delta_width)] if delta_width > 0 else []
		for glyph in self.glyphs.values():
			# Handle width change
			for row in glyph:
				if delta_width < 0:
					del row[delta_width:]
				else:
					row.extend(extra_width.copy())
			# Handle height change
			if delta_height < 0:
				del glyph[delta_height:]
			else:
				glyph.extend([self.new_empty_row() for i in range(delta_height)])

# Common code for all dialogs
class AbstractDialog(Toplevel):
	def __init__(self, master: Misc, buttons_row: int, buttons_columnspan: int):
		super().__init__(master=master)
		# Add OK/Cancel buttons
		buttons = ttk.Frame(master=self)
		ttk.Button(master=buttons, text="Cancel", command=self.on_cancel).grid(row=1, column=1, padx=2, pady=2)
		ttk.Button(master=buttons, text="OK", command=self.on_ok).grid(row=1, column=2, padx=2, pady=2)
		buttons.grid(row=buttons_row, column=1, columnspan=buttons_columnspan)

	def add_size_inputs(self, row: int, focus: bool, width: int = DEFAULT_SIZE, height: int = DEFAULT_SIZE):
		# These variables will get user input
		self.width = IntVar(value=width)
		self.height = IntVar(value=height)
		# Add UI: font width and height, 1st char, last char
		ttk.Label(master=self, text="Width:").grid(row=row, column=1, padx=2, pady=2, sticky="W")
		focus_entry = ttk.Spinbox(master=self, from_=MIN_SIZE, to=MAX_SIZE, increment=1, textvariable=self.width)
		focus_entry.grid(row=row, column=2, padx=2, pady=2)
		if focus:
			focus_entry.focus()
			row += 1
		ttk.Label(master=self, text="Height:").grid(row=row, column=1, padx=2, pady=2, sticky="W")
		ttk.Spinbox(master=self, from_=MIN_SIZE, to=MAX_SIZE, increment=1, textvariable=self.height).grid(
			row=row, column=2, padx=2, pady=2)

	def add_range_inputs(self, row: int, focus: bool, first: int = MIN_CHAR_CODE, last: int = MAX_CHAR_CODE - 1):
		# These variables will get user input
		self.first = StringVar(value=chr(first))
		self.last = StringVar(value=chr(last))
		# Add UI: 1st char, last char
		ttk.Label(master=self, text="First letter:").grid(row=row, column=1, padx=2, pady=2, sticky="W")
		focus_entry = ttk.Spinbox(master=self, values=CHAR_VALUES, textvariable=self.first)
		focus_entry.grid(row=row, column=2, padx=2, pady=2)
		if focus:
			focus_entry.focus()
		row += 1
		ttk.Label(master=self, text="Last letter:").grid(row=row, column=1, padx=2, pady=2, sticky="W")
		ttk.Spinbox(master=self, values=CHAR_VALUES, textvariable=self.last).grid(
			row=row, column=2, padx=2, pady=2)

	def show_modal(self):
		self.protocol(name="WM_DELETE_WINDOW", func=self.on_cancel)
		self.bind('<Escape>', self.on_cancel)
		self.bind('<Return>', self.on_ok)
		self.transient(master=self.master)
		self.wait_visibility()
		self.grab_set()
		self.wait_window()

	def on_ok(self, event = None):
		self.grab_release()
		self.destroy()

	def on_cancel(self, event = None):
		self.grab_release()
		self.destroy()

# Dialog displayed at creation fo a new font
class NewFontDialog(AbstractDialog):
	def __init__(self, master: Misc):
		super().__init__(master=master, buttons_row=5, buttons_columnspan=2)
		self.title("Font Settings")
		self.result: FontState | None = None
		# Add UI: font width and height, 1st char, last char
		self.add_size_inputs(row=1, focus=True)
		self.add_range_inputs(row=3, focus=False)
		self.show_modal()

	def get_font_state(self) -> FontState | None:
		return self.result
	
	def on_ok(self, event = None):
		first = ord(self.first.get())
		last = ord(self.last.get())
		if last < first:
			temp = first
			first = last
			last = temp
		self.result = FontState(name=None, width=self.width.get(), height=self.height.get(),
			first=first, last=last)
		super().on_ok()

@dataclass(kw_only=True)
class ExportConfig:
	fastarduino: bool = False
	vertical: bool = False
	directory: str

# Dialog displayed at export of current font
class ExportDialog(AbstractDialog):
	def __init__(self, master: Misc):
		super().__init__(master=master, buttons_row=4, buttons_columnspan=2)
		self.title("Export Settings")
		self.result: ExportConfig | None = None
		# These variables will get user input
		self.fastarduino = BooleanVar()
		self.vertical = BooleanVar()
		self.directory = StringVar()
		# Add UI: font width and height, 1st char, last char
		focus_entry = ttk.Checkbutton(master=self, text="FastArduino Font", variable=self.fastarduino)
		focus_entry.grid(row=1, column=1, columnspan=2, padx=2, pady=2, sticky="W")
		focus_entry.focus()
		ttk.Checkbutton(master=self, text="Vertical Font", variable=self.vertical).grid(
			row=2, column=1, columnspan=2, padx=2, pady=2, sticky="W")
		ttk.Button(master=self, text="Code Files Directory...", command=self.on_select_dir).grid(
			row=3, column=1, padx=2, pady=2)
		ttk.Label(master=self, textvariable=self.directory).grid(row=3, column=2, padx=2, pady=2)

		self.show_modal()

	def get_export_config(self):
		return self.result
	
	def on_select_dir(self):
		directory = filedialog.askdirectory(title="Select directory to save source code files")
		if directory:
			self.directory.set(directory)
	
	def on_ok(self, event = None):
		# Check that directory is selected
		if not self.directory.get():
			messagebox.showwarning(title="Warning", 
				message="Please choose a directory to which source code files will be saved.")
			return
		self.result = ExportConfig(fastarduino=self.fastarduino.get(), vertical=self.vertical.get(),
			directory=self.directory.get())
		super().on_ok()

# Dialog displayed when changing current font range
class ChangeFontRangeDialog(AbstractDialog):
	def __init__(self, master: Misc, first: int, last: int):
		super().__init__(master=master, buttons_row=3, buttons_columnspan=2)
		self.title("Change Font Range")
		self.result: tuple[int, int] = None
		# Add UI: 1st char, last char
		self.add_range_inputs(row=1, focus=True, first=first, last=last)
		self.show_modal()

	def get_new_range(self):
		return self.result

	def on_ok(self, event = None):
		first = ord(self.first.get())
		last = ord(self.last.get())
		if last < first:
			temp = first
			first = last
			last = temp
		self.result = (first, last)
		super().on_ok()

# Dialog displayed when changing current font size
class ChangeFontSizeDialog(AbstractDialog):
	def __init__(self, master: Misc, width: int, height: int):
		super().__init__(master=master, buttons_row=3, buttons_columnspan=2)
		self.title("Change Font Size")
		self.result: tuple[int, int] = None
		# Add UI: 1st char, last char
		self.add_size_inputs(row=1, focus=True, width=width, height=height)
		self.show_modal()

	def get_new_size(self):
		return self.result

	def on_ok(self, event = None):
		width = self.width.get()
		height = self.height.get()
		self.result = (width, height)
		super().on_ok()

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
		return [[pixel.get_value() for pixel in pixels_row] for pixels_row in self.pixels]

	def update_pixels_from_glyph(self, glyph: list[list[bool]]):
		for glyph_row, pixels_row in zip(glyph, self.pixels):
			for glyph_pixel, pixel in zip(glyph_row, pixels_row):
				pixel.set_value(glyph_pixel)

	def clear_pixels(self):
		for pixels_row in self.pixels:
			for pixel in pixels_row:
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
		self.char: int = None
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
	
	def get_character(self) -> int:
		return self.char
	
	def set_character(self, char: int):
		self.char = char
		self.letter_label.config(text=f"{char:02x}-{chr(char)}")
	
	def empty_glyph(self) -> list[list[str]]:
		data: list[list[str]] = []
		for r in range(self.glyph_height * CharacterThumbnail.PIX_SIZE):
			row = []
			for c in range(self.glyph_width * CharacterThumbnail.PIX_SIZE):
				row.append("white")
			data.append(row)
		return data

	def clear_glyph(self):
		self.glyph_image.put(data=self.empty_glyph())
	
	def set_glyph(self, glyph: list[list[bool]]):
		data = self.empty_glyph()
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

	def __init__(self, master: Misc, font_state: FontState) -> None:
		super().__init__(master, background="white")
		# create CharacterThumbnail for each character in the font
		self.thumbnails: dict[int, CharacterThumbnail] = {}
		self.selected_thumbnail: CharacterThumbnail = None
		gridx = 1
		gridy = 1
		for c in range(font_state.first, font_state.last + 1):
			thumb = CharacterThumbnail(master=self, width=font_state.width, height=font_state.height)
			thumb.bind(sequence="<Button-1>", func=master.on_thumbnail_click)
			thumb.set_character(char=c)
			thumb.set_glyph(glyph=font_state.glyphs[c])
			self.thumbnails[c] = thumb
			# Add thumbnail to the grid
			thumb.grid(row=gridy, column=gridx)
			gridx += 1
			if gridx > ThumbnailPanel.MAX_GRIDX:
				gridx = 1
				gridy += 1
	
	def select_character(self, c: int) -> None:
		if self.selected_thumbnail:
			self.selected_thumbnail.set_highlight(highlight=False)
		self.selected_thumbnail = self.thumbnails[c]
		self.selected_thumbnail.set_highlight(highlight=True)
	
	def update_character(self, c: int, glyph: list[list[bool]]) -> None:
		self.thumbnails[c].set_glyph(glyph=glyph)
	
	def update_all(self, font_state: FontState) -> None:
		for c, glyph in font_state.glyphs.items():
			self.thumbnails[c].set_glyph(glyph=glyph)

class FontEditor(ttk.Frame):
	def __init__(self, master: Tk):
		super().__init__(master, padding=(4, 4, 4, 4))
		master.option_add("*tearOff", False)
		master.minsize(width=300, height=200)

		self.filename: str = None
		self.font_state: FontState = None
		self.previous_char: int = None
		self.thumbnails: ThumbnailPanel = None
		self.glyph_editor: GlyphEditor = None
		self.is_dirty: bool = False
		self.clipboard: list[list[bool]] = None

		# Add menu bar here
		menubar = Menu(master)
		master['menu'] = menubar
		menu_file = Menu(menubar)
		menu_edit = Menu(menubar)
		menubar.add_cascade(menu=menu_file, label="File", underline=0)
		menubar.add_cascade(menu=menu_edit, label="Edit", underline=0)

		menu_file.add_command(label="New Font...", command=self.on_new, underline=0, accelerator="Ctrl+N")
		master.bind('<Control-n>', self.on_new)
		menu_file.add_command(label="Open Font...", command=self.on_open, underline=0, accelerator="Ctrl+O")
		master.bind('<Control-o>', self.on_open)
		menu_file.add_separator()
		menu_file.add_command(label="Save", command=self.on_save, underline=0, accelerator="Ctrl+S")
		master.bind('<Control-s>', self.on_save)
		menu_file.add_command(label="Export...", command=self.on_export, underline=1)
		menu_file.add_command(label="Revert Font", command=self.on_revert_font, underline=0)
		menu_file.add_separator()
		menu_file.add_command(label="Close Font", command=self.on_close, underline=0)
		menu_file.add_separator()
		menu_file.add_command(label="Quit", command=self.on_quit, underline=0, accelerator="Ctrl+Q")
		master.bind('<Control-q>', self.on_quit)

		menu_edit.add_command(label="Revert Glyph", command=self.on_revert_glyph, underline=0)
		menu_edit.add_separator()
		menu_edit.add_command(label="Copy", command=self.on_copy, underline=0, accelerator="Ctrl+C")
		master.bind('<Control-c>', self.on_copy)
		menu_edit.add_command(label="Paste", command=self.on_paste, underline=0, accelerator="Ctrl+V")
		master.bind('<Control-v>', self.on_paste)
		menu_edit.add_separator()
		menu_edit.add_command(label="Change Font Range...", command=self.on_change_font_range, underline=15)
		menu_edit.add_command(label="Change Font Size...", command=self.on_change_font_size, underline=13)

		self.grid(column=0, row=0, sticky=(N))
		master.columnconfigure(0, weight=1)
		master.rowconfigure(0, weight=1)

		self.update_title()
		master.protocol(name="WM_DELETE_WINDOW", func=self.on_quit)

	def update_title(self):
		state = self.font_state
		if state:
			title = f"Editor for font `{state.name}` ({state.width}x{state.height})"
			if self.is_dirty:
				title += " *"
			self.master.title(title)
		else:
			self.master.title("Editor for FastArduino fonts")
	
	def set_font(self, font_state: FontState):
		self.font_state = font_state
		self.previous_char: int = None
		self.is_dirty = False

		thumbnails = self.thumbnails
		glyph_editor = self.glyph_editor

		# Add new thumbnail pane
		self.thumbnails = ThumbnailPanel(master=self, font_state=font_state)
		# Add new panel for glyph editing
		self.glyph_editor = GlyphEditor(
			master=self, width=self.font_state.width, height=self.font_state.height)

		# Replace existing panes
		if thumbnails:
			thumbnails.grid_forget()
			thumbnails.destroy()
		self.thumbnails.grid(row=1, column=1, padx=3, pady=3)
		if glyph_editor:
			glyph_editor.grid_forget()
			glyph_editor.destroy()
		self.glyph_editor.grid(row=1, column=2, padx=3, pady=3)

		# select 1st thumbnail & update title
		self.select_first()
		self.update_title()

	def select_first(self):
		# select 1st thumbnail
		self.click_thumbnail(self.thumbnails.thumbnails[self.font_state.first])

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
		result = messagebox.askyesnocancel(title="Question", 
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

	def on_new(self, event = None):
		# Check if save needed
		if not self.check_dirty(): return
		# Open dialog to select font size and font range
		dialog = NewFontDialog(master=self.master)
		# Get new font info (or None if dialog cancelled)
		font_state = dialog.get_font_state()
		if font_state:
			self.filename = None
			self.set_font(font_state)
			self.is_dirty = True
	
	def on_open(self, event = None):
		# Check if save needed
		if not self.check_dirty(): return
		filename = filedialog.askopenfilename(
			title="Select Font File to Open" ,filetypes=[("Font files", "*.font")])
		if filename:
			self.filename = filename
			# Read FontState from storage and update UI
			with open(filename, 'rb') as input:
				font_state: FontState = pickle.load(input)
				self.set_font(font_state)
	
	def on_close(self):
		# Check if save needed
		if not self.check_dirty(): return
		self.filename = None
	
	def on_save(self, event = None):
		# Check if this is a new font (need to use save dialog)
		if not self.filename:
			# Open save file dialog
			filename = filedialog.asksaveasfilename(
				title="Save Font as", filetypes=[("Font files", "*.font")])
			# Update font_state name according to selected filename
			if not filename:
				return
			matcher = re.search(r"([^/\\]*)\.font$", filename)
			if not matcher:
				print(f"{filename} does not match regex pattern (.font extension)")
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
		self.is_dirty = False
		self.update_title()

	def on_revert_font(self):
		if not self.filename:
			messagebox.showinfo(title="Operation Impossible", 
		    	message="Impossible to revert until font has been saved once.")
			return
		# Read FontState from storage and update UI
		with open(self.filename, 'rb') as input:
			font_state: FontState = pickle.load(input)
			self.set_font(font_state)

	def on_export(self):
		# Check all characters have been defined in font state
		undefined_glyphs: list[int] = []
		for c, glyph in self.font_state.glyphs.items():
			if not glyph:
				print(f"Glyph for character '{chr(c)}' (0x{c:02x}) is undefined!")
				undefined_glyphs.append(c)
		if len(undefined_glyphs) > 1:
			messagebox.showerror(title="Operation Impossible", 
				message=f"Glyphs for {len(undefined_glyphs)} characters are undefined!")
			return
		if len(undefined_glyphs) > 0:
			c = undefined_glyphs[0]
			messagebox.showerror(title="Operation Impossible", 
				message=f"Glyph for character '{chr(c)}' (0x{c:02x}) is undefined!")
			return
		dialog = ExportDialog(master=self.master)
		export_config = dialog.get_export_config()
		if export_config:
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
	
	def on_quit(self, event = None):
		# Check if save needed
		if not self.check_dirty(): return
		self.master.destroy()
	
	def on_copy(self, event = None):
		# Get current character glyph and copy it to self.clipboard (deep copy)
		self.clipboard = self.glyph_editor.get_glyph_from_pixels()
	
	def on_paste(self, event = None):
		if self.clipboard:
			# Deep copy clipboard content to current character glyph
			self.glyph_editor.update_pixels_from_glyph(self.clipboard)
			# Update thumbnail
			self.thumbnails.update_character(self.previous_char, self.clipboard)
	
	def on_revert_glyph(self):
		# We must read previuous glyph from font file
		if not self.filename:
			messagebox.showinfo(title="Operation Impossible", 
		    	message="Impossible to revert until font open and saved once.")
			return
		with open(self.filename, "rb") as input:
			font_state: FontState = pickle.load(input)
			c: int = self.previous_char
			glyph = font_state.glyphs[c]
			if self.glyph_editor.get_glyph_from_pixels() != glyph:
				self.glyph_editor.update_pixels_from_glyph(glyph=glyph)
				self.update_is_dirty()

	def on_change_font_range(self):
		if not self.filename:
			messagebox.showinfo(title="Operation Impossible",
		    	message="Impossible change range until font open and saved once.")
			return
		first = self.font_state.first
		last = self.font_state.last
		dialog = ChangeFontRangeDialog(master=self.master, first=first, last=last)
		range = dialog.get_new_range()
		if range and range != (first, last):
			# Range has changed => modify font_state and reinit UI
			self.font_state.update_range(first=range[0], last=range[1])
			if self.previous_char < first:
				self.previous_char = first
			elif self.previous_char > last:
				self.previous_char = last
			self.set_font(font_state=self.font_state)
			self.is_dirty = True

	def on_change_font_size(self):
		if not self.filename:
			messagebox.showinfo(title="Operation Impossible",
		    	message="Impossible change size until font open and saved once.")
			return
		width = self.font_state.width
		height = self.font_state.height
		dialog = ChangeFontSizeDialog(master=self.master, width=width, height=height)
		size = dialog.get_new_size()
		if size and size != (width, height):
			# Size has changed => modify font_state and reinit UI
			self.font_state.update_size(width=size[0], height=size[1])
			self.set_font(font_state=self.font_state)
			self.is_dirty = True

FASTARDUINO_HEADER_LICENSE = """//   Copyright 2016-2023 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
"""

FASTARDUINO_HEADER_TEMPLATE = FASTARDUINO_HEADER_LICENSE + """
/// @cond api

#ifndef {font_header_define}
#define {font_header_define}

#include "../font.h"

namespace devices::display
{{
	class {font_name} : public Font<{vertical}>
	{{
	public:
		{font_name}() : Font{{0x{first_char:02x}, 0x{last_char:02x}, {font_width}, {font_height}, FONT}} {{}}

	private:
		static const uint8_t FONT[] PROGMEM;
	}};
}}
#endif /* {font_header_define} */

/// @endcond
"""

FASTARDUINO_SOURCE_TEMPLATE = FASTARDUINO_HEADER_LICENSE + """
#include "{font_header}"

const uint8_t devices::display::{font_name}::FONT[] PROGMEM =
{{
{font_glyphs}}};
"""

REGULAR_CODE_TEMPLATE = """
#include <fastarduino/devices/font.h>

class {font_name} : public devices::display::Font<{vertical}>
{{
public:
	{font_name}() : Font{{0x{first_char:02x}, 0x{last_char:02x}, {font_width}, {font_height}, FONT}} {{}}

private:
	static const uint8_t FONT[] PROGMEM;
}};

const uint8_t {font_name}::FONT[] PROGMEM =
{{
{font_glyphs}}};
"""

#FIXME if glyph_car is \ then it shall be escaped!
GLYPH_TEMPLATE = """	{glyph_row}	// 0x{glyph_code:02x} {glyph_char}
"""

def generate_fastarduino_header(filename: str, font_name: str, width: int, height: int, 
	first_char: int, last_char: int, vertical: bool) -> str:
	# generate header as string
	return FASTARDUINO_HEADER_TEMPLATE.format(
		font_header_define = filename.upper() + '_HH',
		font_name = font_name,
		vertical = 'true' if vertical else 'false',
		first_char = first_char,
		last_char = last_char,
		font_width = width,
		font_height =  height)

def generate_glyph_rows(c: int, width: int, height: int, vertical: bool, glyph: list[list[bool]]):
	glyph_rows = ''
	if vertical:
		for row in range(int((height - 1) / 8 + 1)):
			glyph_row = ''
			for col in range(width):
				mask = 1
				byte = 0
				for i in range(8):
					if row * 8 + i == height:
						break
					# print(f"glyph[{row * 8 + i}][{col}]")
					if glyph[row * 8 + i][col]:
						byte |= mask
					mask *= 2
				glyph_row += f'0x{byte:02x}, '
			# Ensure '\' character is not mis-interpreted by C pre-processor by adding a space afterneath
			glyph_char = '\\ (backslash)' if chr(c) == '\\' else chr(c)
			glyph_rows += GLYPH_TEMPLATE.format(glyph_row = glyph_row, glyph_code = c, glyph_char = glyph_char)
	else:
		#TODO
		pass
	return glyph_rows

def generate_all_glyphs(width: int, height: int, first_char: int, last_char: int, 
	vertical: bool, glyphs: dict[int, list[list[bool]]]) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = ''
	for c in range(first_char, last_char + 1):
		glyph = glyphs[c]
		all_glyphs += generate_glyph_rows(c, width, height, vertical, glyph)
	return all_glyphs

def generate_fastarduino_source(filename: str, font_name: str, width: int, height: int, 
	first_char: int, last_char: int, vertical: bool, glyphs: dict[int, list[list[bool]]]) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = generate_all_glyphs(width, height, first_char, last_char, vertical, glyphs)
	return FASTARDUINO_SOURCE_TEMPLATE.format(
		font_header = filename + '.h',
		font_name = font_name,
		font_glyphs = all_glyphs)

def generate_regular_code(filename: str, font_name: str, width: int, height: int, 
	first_char: int, last_char: int, vertical: bool, glyphs: dict[int, list[list[bool]]]) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = generate_all_glyphs(width, height, first_char, last_char, vertical, glyphs)
	return REGULAR_CODE_TEMPLATE.format(
		font_name = font_name,
		vertical = 'true' if vertical else 'false',
		first_char = first_char,
		last_char = last_char,
		font_width = width,
		font_height =  height,
		font_glyphs = all_glyphs)

if __name__ == '__main__':
	# Create Window
	root = Tk()
	app = FontEditor(root)
	root.mainloop()
