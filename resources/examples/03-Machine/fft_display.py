"""
Audio FFT spectrum display demo.

Captures mono audio from the onboard codec, runs the hardware FFT, and
renders a real-time spectrum on the display.

You can use this web site to generate test audio samples with specific
frequencies:
https://www.audiocutter.org/tw/frequency-generator
"""

import gc
import os
import sys
import time
import image

from machine import FFT
from media.display import *
from media.media import *
from media.pyaudio import *


DISPLAY_IS_LCD = True
DISPLAY_IS_HDMI = False
DISPLAY_IS_IDE = False


def align_up(value, alignment):
	return (value + alignment - 1) & (~(alignment - 1))


LCD_WIDTH = align_up(800, 16)
LCD_HEIGHT = 480
HDMI_WIDTH = 1920
HDMI_HEIGHT = 1080
IDE_WIDTH = 800
IDE_HEIGHT = 480

SAMPLE_RATE = 44100
FFT_POINTS = 512
FFT_SHIFT = 0x555
FORMAT = paInt16
CHANNELS = 1

COLOR_BG = (0, 0, 0)
COLOR_GRID = (48, 48, 48)
COLOR_AXIS = (110, 110, 110)
COLOR_TEXT = (232, 232, 232)
COLOR_LOW = (0, 255, 0)
COLOR_MID = (255, 255, 0)
COLOR_HIGH = (255, 64, 0)
COLOR_PANEL = (0, 0, 0)

AXIS_TICK_COUNT = 6


def exit_check():
	try:
		os.exitpoint()
		return False
	except KeyboardInterrupt as exc:
		print("user stop:", exc)
		return True


def init_display():
	if DISPLAY_IS_HDMI:
		Display.init(Display.LT9611, width=HDMI_WIDTH, height=HDMI_HEIGHT, to_ide=True)
		return HDMI_WIDTH, HDMI_HEIGHT
	if DISPLAY_IS_LCD:
		Display.init(Display.ST7701, width=LCD_WIDTH, height=LCD_HEIGHT, to_ide=True)
		return LCD_WIDTH, LCD_HEIGHT
	if DISPLAY_IS_IDE:
		Display.init(Display.VIRT, width=IDE_WIDTH, height=IDE_HEIGHT, fps=60, to_ide=True)
		return IDE_WIDTH, IDE_HEIGHT

	raise ValueError("Select one display output")


def color_for_ratio(ratio):
	if ratio < 0.5:
		return COLOR_LOW
	if ratio < 0.8:
		return COLOR_MID
	return COLOR_HIGH


def format_freq_label(freq_hz):
	if freq_hz >= 1000:
		if (freq_hz % 1000) == 0:
			return "%dk" % (freq_hz // 1000)
		return "%.1fk" % (freq_hz / 1000.0)
	return str(freq_hz)


def aggregate_bins(values, out_count):
	if out_count <= 0:
		return []

	src_count = len(values)
	if src_count <= out_count:
		return values[:]

	buckets = []
	for idx in range(out_count):
		start = (idx * src_count) // out_count
		end = ((idx + 1) * src_count) // out_count
		if end <= start:
			end = start + 1

		peak = 0
		for pos in range(start, end):
			val = values[pos]
			if val > peak:
				peak = val
		buckets.append(peak)

	return buckets


def draw_frequency_axis(img, plot_x, plot_y, plot_w, axis_y, label_y, sample_rate):
	nyquist = sample_rate // 2
	last_right = -10000
	font_size = 16 if img.width() < 1000 else 24

	img.draw_line(plot_x, axis_y, plot_x + plot_w - 1, axis_y, color=COLOR_AXIS, thickness=1)

	for idx in range(AXIS_TICK_COUNT):
		x = plot_x + ((plot_w - 1) * idx) // (AXIS_TICK_COUNT - 1)
		freq = (nyquist * idx) // (AXIS_TICK_COUNT - 1)
		label = format_freq_label(freq)
		label_w = max(len(label) * (font_size // 2), font_size)
		label_x = x - (label_w // 2)

		if idx == 0:
			label_x = plot_x
		elif idx == AXIS_TICK_COUNT - 1:
			label_x = plot_x + plot_w - label_w

		if label_x <= last_right + 6:
			continue

		img.draw_line(x, axis_y - 4, x, axis_y + 4, color=COLOR_AXIS, thickness=1)
		img.draw_string_advanced(label_x, label_y, font_size, label, color=COLOR_TEXT)
		last_right = label_x + label_w

	img.draw_string_advanced(plot_x + plot_w - font_size * 2, plot_y, font_size, "Hz", color=COLOR_TEXT)


def draw_spectrum(img, amplitudes, sample_rate, peak_level, fps_value):
	width = img.width()
	height = img.height()

	left = 14 if width >= 640 else 8
	right = 14 if width >= 640 else 8
	top = 40
	bottom = 40
	plot_x = left
	plot_y = top
	plot_w = width - left - right
	plot_h = height - top - bottom - 10
	axis_y = plot_y + plot_h + 6
	label_y = axis_y + 6

	if plot_w <= 8 or plot_h <= 8:
		return

	img.clear()
	img.draw_rectangle(0, 0, width, height, color=COLOR_PANEL, fill=True)

	for step in range(1, 5):
		gy = plot_y + plot_h - (plot_h * step) // 4
		img.draw_line(plot_x, gy, plot_x + plot_w - 1, gy, color=COLOR_GRID, thickness=1)

	visible_bars = min(len(amplitudes), max(1, plot_w // 3))
	bars = aggregate_bins(amplitudes, visible_bars)
	gap = 1
	bar_w = max(1, (plot_w - ((visible_bars - 1) * gap)) // visible_bars)
	if bar_w <= 1:
		gap = 0
		bar_w = max(1, plot_w // visible_bars)

	scale_peak = max(peak_level, 1.0)
	for idx in range(visible_bars):
		ratio = bars[idx] / scale_peak
		if ratio > 1.0:
			ratio = 1.0

		bar_h = int(ratio * plot_h)
		x = plot_x + idx * (bar_w + gap)
		y = plot_y + plot_h - bar_h
		img.draw_rectangle(x, y, bar_w, bar_h, color=color_for_ratio(ratio), fill=True)

	draw_frequency_axis(img, plot_x, plot_y, plot_w, axis_y, label_y, sample_rate)
	img.draw_string_advanced(10, 6, 20, "Audio FFT Spectrum", color=COLOR_TEXT)
	img.draw_string_advanced(width - 140, 6, 20, "%.1f fps" % fps_value, color=COLOR_TEXT)


def run_fft_display():
	display_width, display_height = init_display()
	img = image.Image(display_width, display_height, image.ARGB8888)
	pcm_buffer = bytearray(FFT_POINTS * 2)
	fft = FFT(pcm_buffer, FFT_POINTS, FFT_SHIFT)
	pa = PyAudio()
	stream = None
	fps = time.clock()
	peak_level = 100.0
	frame_count = 0

	try:
		stream = pa.open(
			format=FORMAT,
			channels=CHANNELS,
			rate=SAMPLE_RATE,
			input=True,
			frames_per_buffer=FFT_POINTS,
		)
		stream.enable_audio3a(AUDIO_3A_ENABLE_ANS)

		print("fft display start")
		print("sample_rate=%d fft_points=%d display=%dx%d" % (SAMPLE_RATE, FFT_POINTS, display_width, display_height))

		while True:
			fps.tick()
			if exit_check():
				break

			raw = stream.read()
			if not raw:
				continue

			copy_len = min(len(raw), len(pcm_buffer))
			pcm_buffer[:copy_len] = raw[:copy_len]
			if copy_len < len(pcm_buffer):
				pcm_buffer[copy_len:] = b"\x00" * (len(pcm_buffer) - copy_len)

			spectrum = fft.run()
			amplitudes = fft.amplitude(spectrum)
			useful = amplitudes[: FFT_POINTS // 2]
			if len(useful) > 1:
				frame_peak = max(useful[1:])
			else:
				frame_peak = max(useful)

			if frame_peak > peak_level:
				peak_level = frame_peak
			else:
				peak_level = peak_level * 0.95 + frame_peak * 0.05

			draw_spectrum(img, useful, SAMPLE_RATE, peak_level, fps.fps())
			Display.show_image(img)

			frame_count += 1
			if (frame_count % 30) == 0:
				gc.collect()

	except KeyboardInterrupt as exc:
		print("user stop:", exc)
	except BaseException as exc:
		import sys
		sys.print_exception(exc)
	finally:
		try:
			fft.deinit()
		except Exception:
			pass

		if stream is not None:
			try:
				stream.stop_stream()
			except Exception:
				pass
			try:
				stream.close()
			except Exception:
				pass

		pa.terminate()
		Display.deinit()
		os.exitpoint(os.EXITPOINT_ENABLE_SLEEP)
		time.sleep_ms(100)


if __name__ == "__main__":
	os.exitpoint(os.EXITPOINT_ENABLE)
	run_fft_display()
