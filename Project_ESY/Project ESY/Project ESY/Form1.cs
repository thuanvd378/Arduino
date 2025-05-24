using System;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Project_ESY
{
    public partial class Form1 : Form
    {
        SerialPort _serial;
        bool _readySeen = false;

        public Form1()
        {
            InitializeComponent();
            LoadSerialPorts();
        }

        private void LoadSerialPorts()
        {
            cmbPorts.Items.Clear();
            var ports = SerialPort.GetPortNames();
            Array.Sort(ports);
            cmbPorts.Items.AddRange(ports);
            if (ports.Length > 0) cmbPorts.SelectedIndex = 0;
        }

        private void btnRefresh_Click(object sender, EventArgs e) => LoadSerialPorts();

        private void btnConnect_Click(object sender, EventArgs e)
        {
            if (_serial != null && _serial.IsOpen)
            {
                Disconnect();
                btnConnect.Text = "Kết nối";
                return;
            }

            if (cmbPorts.SelectedItem == null) return;
            string portName = cmbPorts.SelectedItem.ToString();
            _serial = new SerialPort(portName, 115200)
            {
                NewLine = "\n",
                DtrEnable = false,
                RtsEnable = false,
                ReadTimeout = 1000,
                WriteTimeout = 1000
            };
            _serial.DataReceived += Serial_DataReceived;
            try
            {
                _serial.Open();
                btnConnect.Text = "Ngắt kết nối";
                Task.Delay(2000).ContinueWith(_ =>
                {
                    if (_serial.IsOpen && !_readySeen)
                        SafeWriteAndEcho("GETDATA\n");
                });
            }
            catch (UnauthorizedAccessException)
            {
                MessageBox.Show("Cổng đang bị ứng dụng khác giữ, hãy đóng ứng dụng đó.",
                                "Access denied", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Disconnect();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Không thể mở cổng: " + ex.Message);
                Disconnect();
            }
        }

        private void Disconnect()
        {
            if (_serial != null)
            {
                _serial.DataReceived -= Serial_DataReceived;
                if (_serial.IsOpen) _serial.Close();
                _serial.Dispose();
                _serial = null;
            }
        }

        private void SafeWrite(string s)
        {
            try { if (_serial?.IsOpen == true) _serial.Write(s); }
            catch { /* ignore */ }
        }

        private void Serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string line = _serial.ReadLine().Trim();
                AppendConsole(line);
                if (line == "READY") { _readySeen = true; SafeWriteAndEcho("GETDATA\n"); return; }
                if (line == "END_OF_DATA" || line.StartsWith("ERR")) return;

                var parts = line.Split(',');
                if (parts.Length == 2)
                {
                    string id = parts[0];
                    string time = parts[1];
                    Invoke(new Action(() => dgvData.Rows.Add(id, time)));
                }
            }
            catch (TimeoutException) { /* ignore */ }
            finally{}
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            Disconnect();
            base.OnFormClosing(e);
        }
        private void AppendConsole(string text)
        {
            if (rtbConsole.InvokeRequired)
            {
                rtbConsole.Invoke(new Action<string>(AppendConsole), text);
                return;
            }
            // Giới hạn tối đa 5000 dòng để không phình RAM
            if (rtbConsole.Lines.Length > 5000)
                rtbConsole.Clear();

            rtbConsole.AppendText(text + Environment.NewLine);
            rtbConsole.SelectionStart = rtbConsole.TextLength;
            rtbConsole.ScrollToCaret();
        }

        private void SafeWriteAndEcho(string s)
        {
            AppendConsole("→ " + s.Trim());   // hiển thị lệnh gửi
            SafeWrite(s);
        }
        private void button3_Click(object sender, EventArgs e)
        {
            if (dgvData.Rows.Count == 0)
            {
                MessageBox.Show("Chưa có dữ liệu để xuất.");
                return;
            }
            using (var sfd = new SaveFileDialog { Filter = "CSV file (*.csv)|*.csv", FileName = "log.csv" })
            {
                if (sfd.ShowDialog() != DialogResult.OK) return;
                try
                {
                    using (var sw = new StreamWriter(sfd.FileName, false, Encoding.UTF8))
                    {
                        var headers = dgvData.Columns.Cast<DataGridViewColumn>()
                                          .Select(c => $"\"{c.HeaderText}\"");
                        sw.WriteLine(string.Join(",", headers));
                        foreach (DataGridViewRow row in dgvData.Rows)
                        {
                            if (row.IsNewRow) continue;
                            var cells = row.Cells.Cast<DataGridViewCell>()
                                           .Select(c => c.Value == null ? "" : $"\"{c.Value.ToString().Replace("\"", "\"\"")}\"");
                            sw.WriteLine(string.Join(",", cells));
                        }
                    }
                    MessageBox.Show("Xuất dữ liệu thành công!");
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Lỗi khi xuất: " + ex.Message);
                }
            }
        }
    }
}