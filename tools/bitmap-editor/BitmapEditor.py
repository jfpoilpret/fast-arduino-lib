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
 
# This python mini-application allows creation of display bitmaps and creates CPP
# header from created bitmap.

#TODO export rework
#TODO export to binary file (for use from Flash SD)
from dataclasses import dataclass
import pickle
import re

from tkinter import *
from tkinter import filedialog, messagebox
from tkinter import ttk

MIN_SIZE = 4
DEFAULT_SIZE = 16
MAX_SIZE = 255

# Utility to trick events handling so that events pass through from a widget to a parent widget
def pass_events_to_parent(widget: Widget, parent: Widget, events: list[str]):
	bindtags = list(widget.bindtags())
	bindtags.insert(1, parent)
	widget.bindtags(tuple(bindtags))
	for event in events:
		widget.bind(event, lambda e: None)

# Class embedding bitmap state for persistence
@dataclass(kw_only=True)
class BitmapState:
	name: str
	width: int
	height: int
	bitmap: list[list[bool]] = None

	def new_empty_row(self) ->list[bool]:
		return [False for i in range(self.width)]

	def new_empty_bitmap(self) -> list[list[bool]]:
		return [self.new_empty_row() for i in range(self.height)]

	def update_size(self, width: int, height: int):
		delta_height = height - self.height
		delta_width = width - self.width
		self.width = width
		self.height = height
		extra_width = [False for i in range(delta_width)] if delta_width > 0 else []
		# Handle width change
		for row in self.bitmap:
			if delta_width < 0:
				del row[delta_width:]
			else:
				row.extend(extra_width.copy())
		# Handle height change
		if delta_height < 0:
			del self.bitmap[delta_height:]
		else:
			self.bitmap.extend([self.new_empty_row() for i in range(delta_height)])

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
class NewBitmapDialog(AbstractDialog):
	def __init__(self, master: Misc):
		super().__init__(master=master, buttons_row=3, buttons_columnspan=2)
		self.title("Bitmap Settings")
		self.result: BitmapState | None = None
		# Add UI: font width and height
		self.add_size_inputs(row=1, focus=True)
		self.show_modal()

	def get_bitmap_state(self) -> BitmapState | None:
		return self.result
	
	def on_ok(self, event = None):
		self.result = BitmapState(name=None, width=self.width.get(), height=self.height.get())
		super().on_ok()

@dataclass(kw_only=True)
class ExportConfig:
	directory: str

# Dialog displayed at export of current font
class ExportDialog(AbstractDialog):
	def __init__(self, master: Misc):
		super().__init__(master=master, buttons_row=4, buttons_columnspan=2)
		self.title("Export Settings")
		self.result: ExportConfig | None = None
		# These variables will get user input
		self.directory = StringVar()
		# Add UI: font width and height, 1st char, last char
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
		self.result = ExportConfig(directory=self.directory.get())
		super().on_ok()

# Dialog displayed when changing current bitmap size
class ChangeBitmapSizeDialog(AbstractDialog):
	def __init__(self, master: Misc, width: int, height: int):
		super().__init__(master=master, buttons_row=3, buttons_columnspan=2)
		self.title("Change Bitmap Size")
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

# Widget reprenting the thumbnail for a bitmap
#TODO do we really need this pane, can't we directly use PhotoImage?
class BitmapThumbnail(Frame):
	PIX_SIZE = 2

	def __init__(self, master: Misc, width: int, height: int) -> None:
		super().__init__(master, padx=1, pady=1, border=1, background="white", borderwidth=2, relief='solid')
		self.bitmap_height = height
		self.bitmap_width = width
		self.bitmap_image = PhotoImage(master=self, 
			width=width*BitmapThumbnail.PIX_SIZE, height=height*BitmapThumbnail.PIX_SIZE)
		self.bitmap_label = Label(self, background="white")
		self.bitmap_label.grid(row=1, column=0)
		self.bitmap_label.config(image=self.bitmap_image)
		# Ensure events pass through from labels to parent Frame (self)
		pass_events_to_parent(self.bitmap_label, self, ["<Button-1>"])

	def empty_bitmap(self) -> list[list[str]]:
		data: list[list[str]] = []
		for r in range(self.bitmap_height * BitmapThumbnail.PIX_SIZE):
			row = []
			for c in range(self.bitmap_width * BitmapThumbnail.PIX_SIZE):
				row.append("white")
			data.append(row)
		return data

	def clear_bitmap(self):
		self.bitmap_image.put(data=self.empty_bitmap())
	
	def set_bitmap(self, bitmap: list[list[bool]]):
		data = self.empty_bitmap()
		for r, row in enumerate(bitmap):
			for c, col in enumerate(row):
				if col:
					c1 = c * BitmapThumbnail.PIX_SIZE
					for x in range(c1, c1 + BitmapThumbnail.PIX_SIZE):
						r1 = r * BitmapThumbnail.PIX_SIZE
						for y in range(r1, r1 + BitmapThumbnail.PIX_SIZE):
							data[y][x] = "black"
		self.bitmap_image.put(data=data)

# Pane for editing a bitmap
class BitmapPixelsEditor(ttk.Frame):
	def __init__(self, master: Misc, width: int, height: int, thumbnail: BitmapThumbnail) -> None:
		super().__init__(master=master)
		self.width = width
		self.height = height
		self.thumbnail = thumbnail
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

	def get_bitmap_from_pixels(self) -> list[list[bool]]:
		return [[pixel.get_value() for pixel in pixels_row] for pixels_row in self.pixels]

	def update_pixels_from_bitmap(self, bitmap: list[list[bool]]):
		for bitmap_row, pixels_row in zip(bitmap, self.pixels):
			for bitmap_pixel, pixel in zip(bitmap_row, pixels_row):
				pixel.set_value(bitmap_pixel)

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
		self.thumbnail.set_bitmap(self.get_bitmap_from_pixels())

	def on_pixel_move(self, event: Event):
		pixel: Pixel = event.widget
		pixel = self.find_pixel(event.x + pixel.winfo_x(), event.y + pixel.winfo_y())
		if pixel:
			pixel.set_value(self.default_pixel_value)
			self.thumbnail.set_bitmap(self.get_bitmap_from_pixels())

class BitmapEditor(ttk.Frame):
	def __init__(self, master: Tk):
		super().__init__(master, padding=(4, 4, 4, 4))
		master.option_add("*tearOff", False)
		master.minsize(width=300, height=200)

		self.filename: str = None
		self.bitmap_state: BitmapState = None
		self.thumbnail: BitmapThumbnail = None
		self.bitmap_editor: BitmapPixelsEditor = None
		self.is_dirty: bool = False

		# Add menu bar here
		menubar = Menu(master)
		master['menu'] = menubar
		menu_file = Menu(menubar)
		menu_edit = Menu(menubar)
		menubar.add_cascade(menu=menu_file, label="File", underline=0)
		menubar.add_cascade(menu=menu_edit, label="Edit", underline=0)

		menu_file.add_command(label="New Bitmap...", command=self.on_new, underline=0, accelerator="Ctrl+N")
		master.bind('<Control-n>', self.on_new)
		menu_file.add_command(label="Open Bitmap...", command=self.on_open, underline=0, accelerator="Ctrl+O")
		master.bind('<Control-o>', self.on_open)
		menu_file.add_separator()
		menu_file.add_command(label="Save", command=self.on_save, underline=0, accelerator="Ctrl+S")
		master.bind('<Control-s>', self.on_save)
		menu_file.add_command(label="Export...", command=self.on_export, underline=1)
		menu_file.add_command(label="Revert Bitmap", command=self.on_revert_bitmap, underline=0)
		menu_file.add_separator()
		menu_file.add_command(label="Close Bitmap", command=self.on_close, underline=0)
		menu_file.add_separator()
		menu_file.add_command(label="Quit", command=self.on_quit, underline=0, accelerator="Ctrl+Q")
		master.bind('<Control-q>', self.on_quit)

		menu_edit.add_command(label="Change Bitmap Size...", command=self.on_change_bitmap_size, underline=13)

		self.grid(column=0, row=0, sticky=(N))
		master.columnconfigure(0, weight=1)
		master.rowconfigure(0, weight=1)

		self.update_title()
		master.protocol(name="WM_DELETE_WINDOW", func=self.on_quit)

	def update_title(self):
		state = self.bitmap_state
		if state:
			title = f"Editor for bitmap `{state.name}` ({state.width}x{state.height})"
			if self.is_dirty:
				title += " *"
			self.master.title(title)
		else:
			self.master.title("Editor for FastArduino bitmaps")
	
	def set_bitmap(self, bitmap_state: BitmapState):
		self.bitmap_state = bitmap_state
		self.is_dirty = False

		thumbnail = self.thumbnail
		bitmap_editor = self.bitmap_editor

		# Add new thumbnail pane
		self.thumbnail = BitmapThumbnail(master=self, 
			width=self.bitmap_state.width, height=self.bitmap_state.height)
		self.thumbnail.set_bitmap(bitmap=self.bitmap_state.bitmap)
		# Add new panel for bitmap editing
		self.bitmap_editor = BitmapPixelsEditor(master=self, 
			width=self.bitmap_state.width, height=self.bitmap_state.height, 
			thumbnail=self.thumbnail)
		self.bitmap_editor.update_pixels_from_bitmap(self.bitmap_state.bitmap)

		# Replace existing panes
		if thumbnail:
			thumbnail.grid_forget()
			thumbnail.destroy()
		self.thumbnail.grid(row=1, column=1, padx=3, pady=3)
		if bitmap_editor:
			bitmap_editor.grid_forget()
			bitmap_editor.destroy()
		self.bitmap_editor.grid(row=1, column=2, padx=3, pady=3)

		# update title
		self.update_title()

	def update_is_dirty(self):
		bitmap = self.bitmap_editor.get_bitmap_from_pixels()
		if bitmap != self.bitmap_state.bitmap:
			self.is_dirty = True
			# self.thumbnail.set_bitmap(bitmap)
			self.bitmap_state.bitmap = bitmap
			self.update_title()
	
	def check_dirty(self) -> bool:
		if not self.is_dirty:
			return True
		result = messagebox.askyesnocancel(title="Question", 
			message=f"Font `{self.bitmap_state.name}` has changed.\nDo you want to save it before proceeding?")
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
	
	def on_new(self, event = None):
		# Check if save needed
		if not self.check_dirty(): return
		# Open dialog to select font size and font range
		dialog = NewBitmapDialog(master=self.master)
		# Get new font info (or None if dialog cancelled)
		bitmap_state = dialog.get_bitmap_state()
		if bitmap_state:
			self.filename = None
			self.set_bitmap(bitmap_state)
			self.is_dirty = True
	
	def on_open(self, event = None):
		# Check if save needed
		if not self.check_dirty(): return
		filename = filedialog.askopenfilename(
			title="Select Bitmap File to Open" ,filetypes=[("Bitmap files", "*.bitmap")])
		if filename:
			self.filename = filename
			# Read FontState from storage and update UI
			with open(filename, 'rb') as input:
				bitmap_state: BitmapState = pickle.load(input)
				self.set_bitmap(bitmap_state)
	
	def on_close(self):
		# Check if save needed
		if not self.check_dirty(): return
		self.filename = None
		#TODO Clear display!
	
	def on_save(self, event = None):
		# Check if this is a new font (need to use save dialog)
		if not self.filename:
			# Open save file dialog
			filename = filedialog.asksaveasfilename(
				title="Save Bitmap as", filetypes=[("Bitmap files", "*.bitmap")])
			# Update bitmap_state name according to selected filename
			if not filename:
				return
			matcher = re.search(r"([^/\\]*)\.bitmap$", filename)
			if not matcher:
				print(f"{filename} does not match regex pattern (.bitmap extension)")
				return
			self.filename = filename
			self.bitmap_state.name = matcher.group(1)
			
		# Update bitmap
		bitmap = self.bitmap_editor.get_bitmap_from_pixels()
		self.bitmap_state.bitmap = bitmap
		# self.thumbnail.set_bitmap(bitmap=bitmap)
		# Save font state to storage
		with open(self.filename, 'wb') as output:
			pickle.dump(self.bitmap_state, file = output)
		self.is_dirty = False
		self.update_title()

	def on_revert_bitmap(self):
		if not self.filename:
			messagebox.showinfo(title="Operation Impossible", 
		    	message="Impossible to revert until bitmap has been saved once.")
			return
		# Read FontState from storage and update UI
		with open(self.filename, 'rb') as input:
			bitmap_state: BitmapState = pickle.load(input)
			self.set_bitmap(bitmap_state)

	def on_export(self):
		dialog = ExportDialog(master=self.master)
		export_config = dialog.get_export_config()
		if export_config:
			# Perform export
			directory = export_config.directory
			filename = f"{directory}/{self.bitmap_state.name}"
			# Generate regular source code (for specific program use)
			source = generate_regular_code(filename, self.bitmap_state.name,
				self.bitmap_state.width, self.bitmap_state.height, self.bitmap_state.bitmap)
			with open(filename + '.h', 'wt') as output:
				output.write(source)
	
	def on_quit(self, event = None):
		# Check if save needed
		if not self.check_dirty(): return
		self.master.destroy()
	
	def on_change_bitmap_size(self):
		if not self.filename:
			messagebox.showinfo(title="Operation Impossible",
		    	message="Impossible change size until font open and saved once.")
			return
		width = self.bitmap_state.width
		height = self.bitmap_state.height
		dialog = ChangeBitmapSizeDialog(master=self.master, width=width, height=height)
		size = dialog.get_new_size()
		if size and size != (width, height):
			# Size has changed => modify bitmap_state and reinit UI
			self.bitmap_state.update_size(width=size[0], height=size[1])
			self.set_bitmap(bitmap_state=self.bitmap_state)
			self.is_dirty = True

REGULAR_CODE_TEMPLATE = """
#include <fastarduino/devices/font.h>

static const uint8_t {bitmap_name}[] PROGMEM =
{{
{bitmap_content}}};
"""

BITMAP_ROW_TEMPLATE = """	{bitmap_row}
"""

def generate_glyph_rows(c: int, width: int, height: int, glyph: list[list[bool]]):
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
			glyph_rows += BITMAP_ROW_TEMPLATE.format(glyph_row = glyph_row, glyph_code = c, glyph_char = glyph_char)
	else:
		#TODO
		pass
	return glyph_rows

def generate_regular_code(filename: str, font_name: str, width: int, height: int, 
	bitmap: list[list[bool]]) -> str:
	# First generate all rows for bitmap definition
	all_glyphs = generate_all_glyphs(width, height, bitmap)
	return REGULAR_CODE_TEMPLATE.format(
		font_name = font_name,
		font_width = width,
		font_height =  height,
		font_glyphs = all_glyphs)

if __name__ == '__main__':
	# Create Window
	root = Tk()
	app = BitmapEditor(root)
	root.mainloop()
