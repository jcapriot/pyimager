import pytest
import io
import numpy as np
from seispy.synthetics import spike, synlv
import numpy.testing as npt
import time

def test_write_read_path(tmp_path, f_name='spike.segy'):
    file_path = tmp_path / f_name

    segy1 = spike().to_memory()
    assert segy1.in_memory

    segy2 = spike().to_file(file_path)
    assert segy2.on_disk

    for tr1, tr2 in zip(segy1, segy2):
        npt.assert_equal(np.asarray(tr1), np.asarray(tr2))


def test_write_open_file(tmp_path, f_name='spike.segy'):
    file_path = tmp_path / f_name

    segy1 = spike().to_memory()
    with open(file_path, 'wb') as f:
        segy2 = spike().to_file(f)

    # segy grabbed the file name from the opened object for the future!
    assert segy2.on_disk

    for tr1, tr2 in zip(segy1, segy2):
        npt.assert_equal(np.asarray(tr1), np.asarray(tr2))


def test_write_open_stream(tmp_path, f_name='spike.segy'):
    file_path = tmp_path / f_name

    segy = spike().to_file(file_path)
    segy.to_memory()

    with io.BytesIO() as stream:
        spike().to_memory().to_stream(stream)
        stream_bytes = stream.getvalue()

    with open(file_path, 'rb') as f:
        file_bytes = f.read()

    assert stream_bytes == file_bytes

